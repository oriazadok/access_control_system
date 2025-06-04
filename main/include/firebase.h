#ifndef FIREBASE_H
#define FIREBASE_H

#include "esp_err.h" // For esp_err_t type (ESP-IDF standard error codes)

/**
 * @brief Sign in to Firebase Authentication with email and password.
 *
 * This function performs a sign-in request to Firebase using predefined
 * credentials (email and password) to obtain an ID token (JWT).
 * 
 * The ID token will be used later to authorize access to Firebase services,
 * like Realtime Database writes.
 *
 * @note This function must be called successfully before calling any function
 *       that requires authentication, such as send_rfid_log_to_firebase().
 *
 * @return
 *     - ESP_OK on successful sign-in and token retrieval.
 *     - ESP_FAIL or appropriate error code otherwise.
 */
esp_err_t firebase_sign_in(void);

/**
 * @brief Send an RFID log entry to the Firebase Realtime Database.
 *
 * This function uploads a new RFID scan log, containing the UID and timestamp,
 * to the specified Firebase Realtime Database.
 *
 * @param uid The UID of the scanned RFID tag (as a string).
 * @param timestamp The timestamp string representing when the RFID tag was scanned.
 *
 * @note The user must have successfully signed in using firebase_sign_in() first.
 *
 * @return
 *     - ESP_OK on successful data upload.
 *     - ESP_FAIL if authentication token is missing or upload fails.
 */
esp_err_t send_rfid_log_to_firebase(const char *uid, const char *timestamp);

#endif // FIREBASE_H
