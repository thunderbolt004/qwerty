#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>

class User {
private:
    std::string username;
    std::string passwordHash;
    bool isLogin = false;
    int bestWPM;
    int bestAccuracy;

public:
    User(const std::string& username, const std::string& password, int isStored) : username(username) {
        isLogin = false;
        if (isStored) {
            passwordHash = password;
            std::string userDir = "users/";
            std::string userFile = userDir + username;
            std::ifstream file(userFile);
            std::string line;
            while (std::getline(file, line)) {
                std::istringstream iss(line);
                std::string criterion, value;
                if (iss >> criterion >> value) {
                    if (criterion == "BestWPM:") {
                        bestWPM = stoi(value);
                    }
                    if (criterion == "BestAccuracy:") {
                        bestAccuracy = stoi(value);
                    }
                }
            }
        }
        else {
            encrypt_sha256(password, passwordHash);
        }
    }

    void encrypt_sha256(const std::string& message, std::string& encrypted) {
        EVP_MD_CTX* mdctx;
        const EVP_MD* md = EVP_sha256();

        mdctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(mdctx, md, NULL);
        EVP_DigestUpdate(mdctx, message.c_str(), message.length());

        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int length;

        EVP_DigestFinal_ex(mdctx, hash, &length);
        EVP_MD_CTX_free(mdctx);

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (unsigned int i = 0; i < length; i++) {
            ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
        }
        encrypted = ss.str();
    }

    bool check_password(const std::string& password) {
        User tempUser(username, password, 0);
        return tempUser.passwordHash == passwordHash;
    }

    static const char* greet() {
        const char* greetMessage = "Hello, ";
        return greetMessage;
    }

    const char* get_name() {
        const char* user = username.c_str();
        return user;
    }

    int getWPM() {
        return bestWPM;
    }

    int getAccuracy() {
        return bestAccuracy;
    }

    void update_data(int wpm, int accuracy) {
        if (wpm > bestWPM) {
            bestWPM = wpm;
        }
        if (accuracy > bestAccuracy) {
            bestAccuracy = accuracy;
        }
    }

    void save_to_file(const std::string& filename) const {
        std::ofstream file(filename, std::ios::app);
        if (file.is_open()) {
            file << username << " " << passwordHash << std::endl;
            file.close();
        } else {
            std::cerr << "Unable to open file." << std::endl;
        }
    }

    bool is_login() {
        return isLogin;
    }

    bool login(const std::string& password) {
        if (check_password(password)) {
            isLogin = true;
            return true;
        }
        return false;
    }
        

    static User get_user_from_file(const std::string& filename, const std::string& username) {
        std::ifstream file(filename);
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string storedUsername, storedPasswordHash;
            if (iss >> storedUsername >> storedPasswordHash) {
                if (storedUsername == username) {
                    return User(storedUsername, storedPasswordHash, 1);
                }
            }
        }
        throw std::runtime_error("User not found");
    }

    static bool add_user(const std::string& filename, const std::string& username, const std::string& password) {
        try {
            get_user_from_file(filename, username);
            // std::cerr << "User already exists." << std::endl;
            return false;
        } catch (const std::runtime_error&) {
            User newUser(username, password, 0);
            newUser.save_to_file(filename);
            return true;
        }
    }
};
