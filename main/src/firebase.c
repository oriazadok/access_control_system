#include "firebase.h"              // Include our own header first
#include "firebase_credentials.h"
#include "esp_http_client.h"        // ESP-IDF HTTP client
#include "esp_log.h"                // ESP-IDF Logging
#include <string.h>                 // C Standard library for string handling
#include "cJSON.h"                  // Third-party library for JSON parsing and generation

// Tag used for ESP_LOG messages
static const char *TAG = "firebase";



// Size of the buffer used to store HTTP responses
#define RESPONSE_BUFFER_SIZE 4096

// DigiCert Global Root CA certificate (for HTTPS communication)
static const char *firebase_root_cert = \
"-----BEGIN CERTIFICATE-----\n"\
"MIIFYjCCBEqgAwIBAgIQd70NbNs2+RrqIQ/E8FjTDTANBgkqhkiG9w0BAQsFADBX\n"\
"MQswCQYDVQQGEwJCRTEZMBcGA1UEChMQR2xvYmFsU2lnbiBudi1zYTEQMA4GA1UE\n"\
"CxMHUm9vdCBDQTEbMBkGA1UEAxMSR2xvYmFsU2lnbiBSb290IENBMB4XDTIwMDYx\n"\
"OTAwMDA0MloXDTI4MDEyODAwMDA0MlowRzELMAkGA1UEBhMCVVMxIjAgBgNVBAoT\n"\
"GUdvb2dsZSBUcnVzdCBTZXJ2aWNlcyBMTEMxFDASBgNVBAMTC0dUUyBSb290IFIx\n"\
"MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAthECix7joXebO9y/lD63\n"\
"ladAPKH9gvl9MgaCcfb2jH/76Nu8ai6Xl6OMS/kr9rH5zoQdsfnFl97vufKj6bwS\n"\
"iV6nqlKr+CMny6SxnGPb15l+8Ape62im9MZaRw1NEDPjTrETo8gYbEvs/AmQ351k\n"\
"KSUjB6G00j0uYODP0gmHu81I8E3CwnqIiru6z1kZ1q+PsAewnjHxgsHA3y6mbWwZ\n"\
"DrXYfiYaRQM9sHmklCitD38m5agI/pboPGiUU+6DOogrFZYJsuB6jC511pzrp1Zk\n"\
"j5ZPaK49l8KEj8C8QMALXL32h7M1bKwYUH+E4EzNktMg6TO8UpmvMrUpsyUqtEj5\n"\
"cuHKZPfmghCN6J3Cioj6OGaK/GP5Afl4/Xtcd/p2h/rs37EOeZVXtL0m79YB0esW\n"\
"CruOC7XFxYpVq9Os6pFLKcwZpDIlTirxZUTQAs6qzkm06p98g7BAe+dDq6dso499\n"\
"iYH6TKX/1Y7DzkvgtdizjkXPdsDtQCv9Uw+wp9U7DbGKogPeMa3Md+pvez7W35Ei\n"\
"Eua++tgy/BBjFFFy3l3WFpO9KWgz7zpm7AeKJt8T11dleCfeXkkUAKIAf5qoIbap\n"\
"sZWwpbkNFhHax2xIPEDgfg1azVY80ZcFuctL7TlLnMQ/0lUTbiSw1nH69MG6zO0b\n"\
"9f6BQdgAmD06yK56mDcYBZUCAwEAAaOCATgwggE0MA4GA1UdDwEB/wQEAwIBhjAP\n"\
"BgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBTkrysmcRorSCeFL1JmLO/wiRNxPjAf\n"\
"BgNVHSMEGDAWgBRge2YaRQ2XyolQL30EzTSo//z9SzBgBggrBgEFBQcBAQRUMFIw\n"\
"JQYIKwYBBQUHMAGGGWh0dHA6Ly9vY3NwLnBraS5nb29nL2dzcjEwKQYIKwYBBQUH\n"\
"MAKGHWh0dHA6Ly9wa2kuZ29vZy9nc3IxL2dzcjEuY3J0MDIGA1UdHwQrMCkwJ6Al\n"\
"oCOGIWh0dHA6Ly9jcmwucGtpLmdvb2cvZ3NyMS9nc3IxLmNybDA7BgNVHSAENDAy\n"\
"MAgGBmeBDAECATAIBgZngQwBAgIwDQYLKwYBBAHWeQIFAwIwDQYLKwYBBAHWeQIF\n"\
"AwMwDQYJKoZIhvcNAQELBQADggEBADSkHrEoo9C0dhemMXoh6dFSPsjbdBZBiLg9\n"\
"NR3t5P+T4Vxfq7vqfM/b5A3Ri1fyJm9bvhdGaJQ3b2t6yMAYN/olUazsaL+yyEn9\n"\
"WprKASOshIArAoyZl+tJaox118fessmXn1hIVw41oeQa1v1vg4Fv74zPl6/AhSrw\n"\
"9U5pCZEt4Wi4wStz6dTZ/CLANx8LZh1J7QJVj2fhMtfTJr9w4z30Z209fOU0iOMy\n"\
"+qduBmpvvYuR7hZL6Dupszfnw0Skfths18dG9ZKb59UhvmaSGZRVbNQpsg3BZlvi\n"\
"d0lIKO2d1xozclOzgjXPYovJJIultzkMu34qQb9Sz/yilrbCgj8=\n"\
"-----END CERTIFICATE-----\n";

// Global buffers for HTTP response handling
static char *response_buffer = NULL;
static int response_len = 0;

// Global buffer to store Firebase ID Token (JWT)
static char id_token[2048]; 

/**
 * @brief HTTP event handler for collecting response data.
 *
 * This handler is called automatically by the HTTP client during the request.
 * It accumulates chunks of data into the response buffer.
 */
static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (evt->data && evt->data_len > 0) {
                if (response_len + evt->data_len < RESPONSE_BUFFER_SIZE) {
                    // Copy received data into the response buffer
                    memcpy(response_buffer + response_len, evt->data, evt->data_len);
                    response_len += evt->data_len;
                } else {
                    // Prevent buffer overflow
                    ESP_LOGE(TAG, "Response buffer overflow");
                }
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

/**
 * @brief Sign in to Firebase Authentication using email and password.
 *
 * Sends a POST request to Firebase Authentication REST API to obtain
 * a JWT idToken, which is used for authenticating further requests.
 *
 * @return
 *     - ESP_OK if sign-in was successful and idToken is obtained.
 *     - ESP_ERR_NO_MEM if memory allocation fails.
 *     - ESP_FAIL for HTTP or JSON parsing errors.
 */
esp_err_t firebase_sign_in(void) {
    response_buffer = calloc(1, RESPONSE_BUFFER_SIZE);
    response_len = 0;
    if (response_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for response buffer");
        return ESP_ERR_NO_MEM;
    }

    // Configure the HTTP client
    esp_http_client_config_t config = {
        .url = "https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=" FIREBASE_API_KEY,
        .method = HTTP_METHOD_POST,
        .cert_pem = firebase_root_cert,
        .event_handler = _http_event_handler,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Create JSON payload for the sign-in request
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "email", FIREBASE_EMAIL);
    cJSON_AddStringToObject(root, "password", FIREBASE_PASSWORD);
    cJSON_AddBoolToObject(root, "returnSecureToken", true);

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root); // Free the cJSON object

    // Set HTTP headers and request body
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_str, strlen(json_str));

    // Perform the HTTP request
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP Status = %d", status_code);

        // Parse JSON response to extract idToken
        cJSON *response = cJSON_Parse(response_buffer);
        if (response) {
            cJSON *id_token_item = cJSON_GetObjectItem(response, "idToken");
            if (id_token_item && cJSON_IsString(id_token_item)) {
                strncpy(id_token, id_token_item->valuestring, sizeof(id_token) - 1);
                id_token[sizeof(id_token) - 1] = '\0'; // Ensure null-termination
                // ESP_LOGI(TAG, "ID Token: %s", id_token); // Debug: Commented out for security
            } else {
                ESP_LOGE(TAG, "No idToken in response");
            }
            cJSON_Delete(response); // Free the cJSON object
        } else {
            ESP_LOGE(TAG, "Failed to parse JSON");
        }
    } else {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    }

    // Clean up resources
    esp_http_client_cleanup(client);
    free(json_str);
    free(response_buffer);
    response_buffer = NULL;
    response_len = 0;

    return err;
}

/**
 * @brief Send RFID log data to Firebase Realtime Database.
 *
 * Uploads a UID and timestamp to the Firebase Realtime Database
 * under the "rfid_logs" node. Requires a valid idToken for authentication.
 *
 * @param uid       The UID of the RFID tag as a string.
 * @param timestamp Timestamp string for the log entry.
 *
 * @return
 *     - ESP_OK on success.
 *     - ESP_FAIL if no valid idToken is available.
 *     - ESP_ERR_NO_MEM if memory allocation fails.
 */
esp_err_t send_rfid_log_to_firebase(const char *uid, const char *timestamp) {
    if (strlen(id_token) == 0) {
        ESP_LOGE(TAG, "No ID Token available, sign-in first.");
        return ESP_FAIL;
    }

    // Calculate URL length and allocate memory
    size_t url_len = strlen(FIREBASE_PROJECT_ID) + strlen(id_token) + 128;
    char *url = malloc(url_len);
    if (!url) {
        ESP_LOGE(TAG, "Failed to allocate memory for URL");
        return ESP_ERR_NO_MEM;
    }

    // Format the Firebase Realtime Database URL
    snprintf(url, url_len,
             "https://%s-default-rtdb.firebaseio.com/rfid_logs.json?auth=%s",
             FIREBASE_PROJECT_ID, id_token);

    // Create JSON payload for the RFID log
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "uid", uid);
    cJSON_AddStringToObject(root, "timestamp", timestamp);

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root); // Free the cJSON object

    // Configure the HTTP client
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .cert_pem = firebase_root_cert,
        .buffer_size = 4096,     // Increase buffer size for larger payloads
        .buffer_size_tx = 4096,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_str, strlen(json_str));

    // Perform the HTTP POST request
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "POST Status = %d", status_code);
    } else {
        ESP_LOGE(TAG, "POST request failed: %s", esp_err_to_name(err));
    }

    // Clean up resources
    esp_http_client_cleanup(client);
    free(json_str);
    free(url);

    return err;
}