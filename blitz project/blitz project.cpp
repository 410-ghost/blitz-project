#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <iomanip>
#include <openssl/sha.h>
#include <curl/curl.h>

// Fonction pour calculer le hachage SHA-1 d'un fichier
std::string calculate_sha1(const std::string& file_path) {
    std::ifstream file(file_path, std::ifstream::binary);
    if (!file) {
        return "";
    }

    unsigned char buffer[SHA_DIGEST_LENGTH];
    SHA_CTX shaContext;
    SHA1_Init(&shaContext);

    while (file.good()) {
        file.read(reinterpret_cast<char*>(buffer), sizeof(buffer));
        auto bytesRead = static_cast<std::streamsize>(file.gcount());
        SHA1_Update(&shaContext, buffer, bytesRead);
    }

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1_Final(hash, &shaContext);

    std::stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return ss.str();
}

// Fonction pour récupérer la liste de hachages depuis le site web
std::vector<std::string> get_remote_hashes(const std::string& url) {
    std::vector<std::string> hashes;

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        std::stringstream response_data;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* ptr, size_t size, size_t nmemb, std::stringstream* data) {
            data->write(static_cast<char*>(ptr), size * nmemb);
            return size * nmemb;
            });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            std::string line;
            while (std::getline(response_data, line)) {
                hashes.push_back(line);
            }
        }
        else {
            std::cerr << "Impossible de récupérer la liste de hachages." << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    return hashes;
}

int main() {
    // Chemin du dossier à scanner
    std::string folder_path = "/chemin/vers/votre/dossier";

    // Liste des hachages calculés pour chaque fichier
    std::vector<std::string> computed_hashes;

    // Calculer les hachages SHA-1 de chaque fichier dans le dossier
    for (const auto& entry : std::filesystem::directory_iterator(folder_path)) {
        if (entry.is_regular_file()) {
            computed_hashes.push_back(calculate_sha1(entry.path().string()));
        }
    }

    // URL du site web contenant les hachages attendus (à remplacer par votre URL)
    std::string url = "https://www.example.com/hashes.txt";

    // Obtenir la liste de hachages depuis le site web
    std::vector<std::string> remote_hashes = get_remote_hashes(url);

    // Vérifier les hachages et afficher les fichiers en trop
    std::cout << "Fichiers en trop : " << std::endl;
    for (size_t i = 0; i < computed_hashes.size(); ++i) {
        if (std::find(remote_hashes.begin(), remote_hashes.end(), computed_hashes[i]) == remote_hashes.end()) {
            std::cout << folder_path + "/" + std::to_string(i) << std::endl;
        }
    }

    return 0;
}
