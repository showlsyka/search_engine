#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <regex>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <regex>
#include <boost/locale.hpp>
#include <unordered_set>
#include <boost/algorithm/string.hpp>
#include <tuple>


namespace fs = std::filesystem;

struct FileEntry {
    std::string path;
    std::string name;
    std::string sentence;
};


using WordIndex = std::unordered_map<std::string, std::vector<FileEntry>>;

struct ExtractedData {
    std::vector<std::unordered_map<std::string,std::string>> words;
    std::string name;
    std::vector<std::unordered_map<std::string, std::string>> path;
    std::vector<std::unordered_map<std::string, std::string>> perehod;
};

std::string toLowerCase(const std::string& input) {
    boost::locale::generator gen;
    std::locale loc = gen("ru_RU.UTF-8");

    return boost::locale::to_lower(input, loc);
}


// Функция для получения всех HTML-файлов в директории и поддиректориях
std::vector<fs::path> GetAllHtmlFiles(const fs::path& directory) {
    std::vector<fs::path> html_files;
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Директория не существует или не является директорией: " << directory << std::endl;
        return html_files;
    }

    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".html") {
            html_files.emplace_back(entry.path());
        }
    }

    return html_files;
}

std::unordered_set<std::string> initializeStopWords() {
    return {
        "и", "в", "во", "не", "что", "он", "на", "я", "с", "со",
        "как", "а", "то", "все", "она", "так", "его", "но", "да",
        "ты", "к", "у", "же", "вы", "за", "бы", "по", "только",
        "ее", "мне", "было", "вот", "от", "меня", "еще", "нет",
        "о", "из", "ему", "теперь", "когда", "даже", "ну", "вдруг",
        "ли", "если", "уже", "или", "ни", "быть", "был", "него","trace", "mode","об",
    };
}

std::string RemoveSpecialQuotes(const std::string& input) {
    std::string output = input;



    // Определяем байтовые последовательности для ‘ и ’ в UTF-8
    const std::string left_single_quote = "\xE2\x80\x98";  // ‘
    const std::string right_single_quote = "\xE2\x80\x99";
    const std::string right_single_quote2 = ".";

    // Функция для удаления всех вхождений подстроки из строки
    auto remove_all = [&](const std::string& to_remove) {
        size_t pos = 0;
        while ((pos = output.find(to_remove, pos)) != std::string::npos) {
            output.erase(pos, to_remove.length());
        }
        };

    // Удаляем ‘ и ’
    remove_all(left_single_quote);
    remove_all(right_single_quote);

    return output;
}

std::string RemoveSpecialQuotes2(const std::string& input) {
    std::string output = input;



    // Определяем байтовые последовательности для ‘ и ’ в UTF-8
    const std::string left_single_quote = "\xE2\x80\x98";  // ‘
    const std::string right_single_quote = "\xE2\x80\x99";
    const std::string right_single_quote2 = ".";
    const std::string right_single_quote3 = ",";
    const std::string right_single_quote4 = "_";
    const std::string right_single_quote5 = "-";
    const std::string right_single_quote6 = "'";

    // Функция для удаления всех вхождений подстроки из строки
    auto remove_all = [&](const std::string& to_remove) {
        size_t pos = 0;
        while ((pos = output.find(to_remove, pos)) != std::string::npos) {
            output.erase(pos, to_remove.length());
        }
        };

    // Удаляем ‘ и ’
    remove_all(left_single_quote);
    remove_all(right_single_quote);
    remove_all(right_single_quote2);
    remove_all(right_single_quote3);
    remove_all(right_single_quote4);
    remove_all(right_single_quote5);
    remove_all(right_single_quote6);

    return output;
}

// Функция для проверки наличия цифр в строке
bool ContainsDigit(const std::string& str) {
    return std::all_of(str.begin(), str.end(), ::isdigit);
}


std::string MarkWordsInHtml(const std::string& content) {
    std::string result;
    std::regex word_regex(R"(<h([1-6])>\s*(.*?)\s*</h([1-6])>)", std::regex::icase);
    std::sregex_iterator words_begin = std::sregex_iterator(content.begin(), content.end(), word_regex);
    std::sregex_iterator words_end = std::sregex_iterator();

    bool first_match = true;
    size_t last_pos = 0;
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;

        if (first_match) {
            // Добавляем текст до первого совпадения
            result.append(content, 0, match.position());
            first_match = false;
        }
        else {
            // Добавляем текст между предыдущим и текущим совпадением
            result.append(content, last_pos, match.position() - last_pos);
        }

        std::string inner_text = match[2].str();
        std::istringstream iss(inner_text + "\n");
        while (iss >> inner_text) {

            result.append(" <mark>" + inner_text + "<mark> ");
            // Обновляем позицию последнего обработанного символа 
            last_pos = match.position() + match.length();
        }
    }
    result.append(content, last_pos, content.size() - last_pos);

    return result;
}

std::string removeHtmlTags(const std::string& input) {
    // Удаляем HTML-теги
    std::string output = std::regex_replace(input, std::regex(R"(<[^>]*>)"), " ");

    std::unordered_map<std::string, std::string> htmlEntities = {
    { "&nbsp;", " " },
    { "&lt;", "<" },
    { "&gt;", ">" },
    { "&amp;", "&" },
    { "&quot;", "\"" },
    { "&apos;", "'" },
};

    output = std::regex_replace(output, std::regex(R"([\(\)\[\]\{\}\/])"), " ");

    output = std::regex_replace(output, std::regex(R"(см\.\s*также)", std::regex_constants::icase), " ");

    output = std::regex_replace(output, std::regex(R"(&rsquo;)", std::regex_constants::icase), " ");

for (const auto& entity : htmlEntities) {
    size_t pos = 0;
    while ((pos = output.find(entity.first, pos)) != std::string::npos) {
        output.replace(pos, entity.first.length(), entity.second);
        pos += entity.second.length();
    }
}

output = std::regex_replace(output, std::regex(R"( +)"), " ");
output = std::regex_replace(output, std::regex(R"(^\s+|\s+$)"), ""); // Убираем пробелы в начале и в конце

return output;
}

std::string removeHtmlTags2(const std::string& input) {
    // Удаляем HTML-теги
    std::string output = std::regex_replace(input, std::regex(R"(<[^>]*>)"), " ");

    std::unordered_map<std::string, std::string> htmlEntities = {
    { "&nbsp;", " " },
    { "&lt;", "<" },
    { "&gt;", ">" },
    { "&amp;", "&" },
    { "&quot;", "" },
    { "&apos;", "'" },
    };

    output = std::regex_replace(output, std::regex(R"([\(\)\[\]\{\}\/])"), " ");

    // Удаляем фразу "см. также"
    output = std::regex_replace(output, std::regex(R"(см\.\s*также)", std::regex_constants::icase), " ");

    output = std::regex_replace(output, std::regex(R"(&rsquo;)", std::regex_constants::icase), " ");
    output = std::regex_replace(output, std::regex(R"(rsquo;)", std::regex_constants::icase), " ");
    output = std::regex_replace(output, std::regex(R"(rsquo)", std::regex_constants::icase), " ");

    for (const auto& entity : htmlEntities) {
        size_t pos = 0;
        while ((pos = output.find(entity.first, pos)) != std::string::npos) {
            output.replace(pos, entity.first.length(), entity.second);
            pos += entity.second.length();
        }
    }

    output = std::regex_replace(output, std::regex(R"( +)"), " ");
    output = std::regex_replace(output, std::regex(R"(^\s+|\s+$)"), ""); 

    return output;
}

std::vector<std::string> splitWord(const std::string& word) {
    std::vector<std::string> parts;
    std::string part;
    std::size_t start = 0;
    std::size_t end = 0;

    while ((end = word.find_first_of("-/\\", start)) != std::string::npos) {
        part = word.substr(start, end - start);
        if (!part.empty()) { 
            parts.push_back(part);
        }
        start = end + 1;
    }

    part = word.substr(start);
    if (!part.empty()) {
        parts.push_back(part);
    }


    return parts;
}

std::unordered_map<std::string, std::vector<std::string>> LoadSynonyms(const fs::path& synonyms_file) {
    std::unordered_map<std::string, std::vector<std::string>> synonyms_map;
    std::ifstream file(synonyms_file);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл с синонимами: " << synonyms_file << std::endl;
        return synonyms_map;
    }
    std::string line;
    while (std::getline(file, line)) {
        // Удаляем пробелы в начале и конце строки
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        std::istringstream iss(line);
        std::string word;
        std::vector<std::string> synonyms;
        // Разделяем строку по запятой
        while (std::getline(iss, word, ',')) {
            // Удаляем пробелы вокруг слова и добавляем в вектор синонимов
            word.erase(0, word.find_first_not_of(" \t"));
            word.erase(word.find_last_not_of(" \t") + 1);
            synonyms.push_back(word);
        }
        for (size_t i = 0; i < synonyms.size(); ++i) {
            for (size_t j = 0; j < synonyms.size(); ++j) {
                if (i != j) { 
                    synonyms_map[synonyms[i]].push_back(synonyms[j]);
                }
            }
        }
    }
    file.close();
    return synonyms_map;
}

bool has_suffix(const std::string& word, const std::string& suffix) {
    if (word.length() >= suffix.length()) {
        return (0 == word.compare(word.length() - suffix.length(), suffix.length(), suffix));
    }
    return false;
}

std::string stem(const std::string& word) {
    std::string stemmed = word;

    std::vector<std::string> suffixes = {
        "ами", "ями", "ями", "ами",
        "ами", "уют", "ует", "ают", "ят",
        "вали", "яли", "ывали", "ыяли"
        , "ыва", "ова", "ева", "ева",
        "и", "ы", "а", "я", "у", "ю",
        "ого", "ему", "ему", "ыми", "ими",
        "ую", "юю", "ая", "яя", "ою", "ею","ов"
    };

    // Удаляем суффикс, если он найден
    for (const auto& suffix : suffixes) {
        if (has_suffix(stemmed, suffix)) {
            stemmed = stemmed.substr(0, stemmed.length() - suffix.length());
            break;
        }
    }


    return stemmed;
}

ExtractedData ExtractWordsFromHtml(const fs::path& file_path, const std::unordered_map<std::string, std::vector<std::string>>& synonyms_map) {
    ExtractedData data;
    std::ifstream file(file_path);
    std::ifstream file2(file_path);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл: " << file_path << std::endl;
        return data;
    }
    std::unordered_set<std::string> stopWords = initializeStopWords();
    std::string line;
    // Регулярные выражения для поиска необходимых тегов
    std::regex div_regex(R"(<div\s+class\s*=\s*"typography-h4">\s*(.*?)\s*</div>)", std::regex::icase);
    // Обновленное регулярное выражение для заголовков h1-h6
    std::regex heading_regex(R"(<h([1-6])>\s*(.*?)\s*</h([1-6])>)", std::regex::icase);
    //Регулярное вырадение для посика по тегам
    std::regex razdel_regex(R"('\s*(.*?)#b\s*(.*?)' title='Click for details'><B>\s*(.*?)\s*</B>)", std::regex::icase);
    std::regex sentence_regex(R"((<[^.!?]*[.!?]))");
    std::regex kolon_regex(R"(<A NAME="([^"]*)\"></A>\s*<B>\s*(.*?)\s*</B>\s*</span>)", std::regex::icase);
    std::regex b_regex(R"(<B>(.*?)</B>)", std::regex::icase);

    std::smatch match;
    std::string::const_iterator search_start(line.cbegin());
    size_t line_count = 0;

    while (std::getline(file, line)) {
        // Извлечение текста из <div class="typography-h4">...</div>
        std::string::const_iterator search_start(line.cbegin());
        while (std::regex_search(search_start, line.cend(), match, div_regex)) {
            if (match.size() > 1) {
                std::string text = match[1].str();
                std::istringstream iss(text);
                std::string word;
                std::string low_word;
                std::string sentence;

                sentence = removeHtmlTags(line);

                    while (iss >> low_word) {
                        // Приводим слово к нижнему регистру
                        word = toLowerCase(low_word);
                        word.erase(std::find_if(word.rbegin(), word.rend(),
                            [](unsigned char c) { return !std::ispunct(c); }).base(), word.end());
                        // Удаляем пунктуацию с конца и начала слова
                        word.erase(word.begin(), std::find_if(word.begin(), word.end(),
                            [](unsigned char c) { return !std::ispunct(c); }));
                        word.erase(std::find_if(word.rbegin(), word.rend(),
                            [](unsigned char c) { return !std::ispunct(c); }).base(), word.end());
                        std::vector<std::string> tire = splitWord(word);
                        for (std::string word1 : tire) {
                            if (!word1.empty()) {
                                if (stopWords.find(word1) != stopWords.end() || (ContainsDigit(word1))) {
                                    continue;
                                }
                                word1 = stem(word1);
                                std::unordered_map<std::string, std::string> word_map;
                                word_map[word1] = sentence + "...";
                                data.words.push_back(word_map);

                                if (synonyms_map.find(word1) != synonyms_map.end()) {
                                    for (std::string synonym : synonyms_map.at(word1)) {
                                        synonym = stem(synonym);
                                        std::unordered_map<std::string, std::string> synonym_map;
                                        synonym_map[synonym] = sentence + "...";
                                        data.words.push_back(synonym_map);
                                    }
                                }
                            }
                        }
                }
            }
            search_start = match.suffix().first;
        }

        // Извлечение текста из всех заголовков <h1>...</h1> до <h6>...</h6>
        search_start = line.cbegin();
        while (std::regex_search(search_start, line.cend(), match, heading_regex)) {
            if (match.size() > 1) {
                std::string title1 = match[2].str();
                std::string title = title1;
                std::istringstream iss(title);
                std::string sentence;

                sentence = removeHtmlTags(line);

                title.erase(title.begin(), std::find_if(title.begin(), title.end(),
                    [](unsigned char c) { return !std::ispunct(c); }));

                title.erase(std::find_if(title.rbegin(), title.rend(),
                    [](unsigned char c) { return !std::ispunct(c); }).base(), title.end());
                title.erase(title.begin(), std::find_if(title.begin(), title.end(),
                    [](unsigned char c) { return !std::isspace(c); }));
                title.erase(std::find_if(title.rbegin(), title.rend(),
                    [](unsigned char c) { return !std::isspace(c); }).base(), title.end());
                if (!title.empty()) {
                    data.name = title;
                    std::string title3 = title;
                    title = RemoveSpecialQuotes(toLowerCase(title3));
                    std::unordered_map<std::string, std::string> word_map1;
                    word_map1[title] = sentence + "...";
                    data.words.push_back(word_map1);
                }

                while (iss >> title) {
                    std::string title2 = title;
                    title = RemoveSpecialQuotes(toLowerCase(title2));
                    title.erase(title.begin(), std::find_if(title.begin(), title.end(),
                        [](unsigned char c) { return !std::ispunct(c); }));

                    title.erase(std::find_if(title.rbegin(), title.rend(),
                        [](unsigned char c) { return !std::ispunct(c); }).base(), title.end());
                    // Удаляем лишние пробелы
                    title.erase(title.begin(), std::find_if(title.begin(), title.end(),
                        [](unsigned char c) { return !std::isspace(c); }));
                    title.erase(std::find_if(title.rbegin(), title.rend(),
                        [](unsigned char c) { return !std::isspace(c); }).base(), title.end());
                    std::vector<std::string> tire = splitWord(title);
                    for (std::string& title1 : tire) {
                        if (!title1.empty()) {
                            if (stopWords.find(title1) != stopWords.end() || (ContainsDigit(title1))) {
                                continue;
                            }
                            title1 = stem(title1);
                            std::unordered_map<std::string, std::string> word_map;
                            word_map[title1] = sentence + "...";
                            data.words.push_back(word_map);

                            if (synonyms_map.find(title1) != synonyms_map.end()) {
                                for (auto synonym : synonyms_map.at(title1)) {
                                    synonym = stem(synonym);
                                    std::unordered_map<std::string, std::string> synonym_map;
                                    synonym_map[synonym] = sentence + "...";
                                    data.words.push_back(synonym_map);
                                }
                            }
                        }
                    }
                }
            }
            search_start = match.suffix().first;
        }

        search_start = line.cbegin();
        while (std::regex_search(search_start, line.cend(), match, razdel_regex)) {
            if (match.size() > 1) {
                std::string razdel1 = match[3].str();
                std::string razdel = RemoveSpecialQuotes2(toLowerCase(razdel1));
                std::unordered_map<std::string, std::string> map_path2;
                map_path2[razdel] = match[1].str() + "#b" + match[2].str();
                data.perehod.push_back(map_path2);
                std::istringstream iss(razdel1);
                std::string sentence;

                sentence = removeHtmlTags(line);

                if (!razdel.empty()) {
                    std::string copy = razdel;
                    razdel = RemoveSpecialQuotes2(toLowerCase(copy));
                    std::unordered_map<std::string, std::string> word_map2;
                    word_map2[razdel] = sentence + "...";
                    data.words.push_back(word_map2);
                }
                while (iss >> razdel) {
                    std::string razdel2 = razdel;
                    razdel = RemoveSpecialQuotes2(toLowerCase(razdel2));
                    razdel.erase(razdel.begin(), std::find_if(razdel.begin(), razdel.end(),
                        [](unsigned char c) { return !std::ispunct(c); }));
                    razdel.erase(std::find_if(razdel.rbegin(), razdel.rend(),
                        [](unsigned char c) { return !std::ispunct(c); }).base(), razdel.end());
                    razdel.erase(razdel.begin(), std::find_if(razdel.begin(), razdel.end(),
                        [](unsigned char c) { return !std::isspace(c); }));
                    razdel.erase(std::find_if(razdel.rbegin(), razdel.rend(),
                        [](unsigned char c) { return !std::isspace(c); }).base(), razdel.end());
                    std::vector<std::string> tire = splitWord(razdel);
                    for (std::string& razdel0 : tire) {
                        if (!razdel0.empty()) {
                            if (stopWords.find(razdel0) != stopWords.end() || (ContainsDigit(razdel0))) {
                                continue;
                            }
                            razdel0 = stem(razdel0);
                            std::unordered_map<std::string, std::string> word_map;
                            word_map[razdel0] = sentence + "...";
                            data.words.push_back(word_map);
                            map_path2[razdel0] = match[1].str() + "#b" + match[2].str();
                            data.perehod.push_back(map_path2);

                            if (synonyms_map.find(razdel0) != synonyms_map.end()) {
                                for (auto synonym : synonyms_map.at(razdel0)) {
                                    synonym = stem(synonym);
                                    std::unordered_map<std::string, std::string> synonym_map;
                                    synonym_map[synonym] = sentence + "...";
                                    data.words.push_back(synonym_map);
                                }
                            }
                        }
                    }
                }
            }
            search_start = match.suffix().first;
        }

        search_start = line.cbegin();
        while (std::regex_search(search_start, line.cend(), match, b_regex)) {
            if (match.size() > 1) {
                std::string razdel1 = match[1].str();
                std::string razdel = RemoveSpecialQuotes2(removeHtmlTags2(toLowerCase(razdel1)));
                std::istringstream iss(razdel1);
                std::string sentence;

                sentence = removeHtmlTags(line);

                while (iss >> razdel) {
                    std::string razdel2 = razdel;
                    razdel = RemoveSpecialQuotes2(removeHtmlTags2(toLowerCase(razdel2)));
                    razdel.erase(razdel.begin(), std::find_if(razdel.begin(), razdel.end(),
                        [](unsigned char c) { return !std::ispunct(c); }));
                    razdel.erase(std::find_if(razdel.rbegin(), razdel.rend(),
                        [](unsigned char c) { return !std::ispunct(c); }).base(), razdel.end());
                    razdel.erase(razdel.begin(), std::find_if(razdel.begin(), razdel.end(),
                        [](unsigned char c) { return !std::isspace(c); }));
                    razdel.erase(std::find_if(razdel.rbegin(), razdel.rend(),
                        [](unsigned char c) { return !std::isspace(c); }).base(), razdel.end());
                    std::vector<std::string> tire = splitWord(razdel);
                    for (std::string& razdel0 : tire) {
                          if (!razdel0.empty()) {
                            if (stopWords.find(razdel0) != stopWords.end() || (ContainsDigit(razdel0))) {
                                continue;
                            }
                            razdel0 = stem(razdel0);
                            std::unordered_map<std::string, std::string> word_map;
                            word_map[razdel0] = sentence + "...";
                            data.words.push_back(word_map);

                            if (synonyms_map.find(razdel0) != synonyms_map.end()) {
                                for (auto synonym : synonyms_map.at(razdel0)) {
                                    synonym = stem(synonym);
                                    std::unordered_map<std::string, std::string> synonym_map;
                                    synonym_map[synonym] = sentence + "...";
                                    data.words.push_back(synonym_map);
                                }
                            }
                        }
                    }
                }
            }
            search_start = match.suffix().first;

        }
        
    

        ++line_count;
    }
    std::string content((std::istreambuf_iterator<char>(file2)), std::istreambuf_iterator<char>()); // Чтение всего содержимого файла

    search_start = content.cbegin();
    while (std::regex_search(search_start, content.cend(), match, kolon_regex)) {
        if (match.size() > 1) {
            std::string razdel1 = match[2].str() + match[3].str() + match[4].str();
            std::string razdel = RemoveSpecialQuotes2(removeHtmlTags2(toLowerCase(razdel1)));
            std::unordered_map<std::string, std::string> map_path;
            map_path[razdel] = "#" + match[1].str();
            data.path.push_back(map_path);
            std::istringstream iss(razdel1);
            std::string sentence;

            sentence = removeHtmlTags2(razdel);

            if (!razdel.empty()) {
                std::string copy = razdel;
                razdel = RemoveSpecialQuotes2(removeHtmlTags2(toLowerCase(copy)));
                std::unordered_map<std::string, std::string> word_map2;
                word_map2[razdel] = sentence + "...";
                data.words.push_back(word_map2);
            }
            // Удаляем лишние пробелы
            while (iss >> razdel) {
                std::string razdel2 = razdel;
                razdel = RemoveSpecialQuotes2(removeHtmlTags2(toLowerCase(razdel2)));
                razdel.erase(razdel.begin(), std::find_if(razdel.begin(), razdel.end(),
                    [](unsigned char c) { return !std::ispunct(c); }));
                razdel.erase(std::find_if(razdel.rbegin(), razdel.rend(),
                    [](unsigned char c) { return !std::ispunct(c); }).base(), razdel.end());
                razdel.erase(razdel.begin(), std::find_if(razdel.begin(), razdel.end(),
                    [](unsigned char c) { return !std::isspace(c); }));
                razdel.erase(std::find_if(razdel.rbegin(), razdel.rend(),
                    [](unsigned char c) { return !std::isspace(c); }).base(), razdel.end());
                std::vector<std::string> tire = splitWord(razdel);
                for (std::string& razdel0 : tire) {
                    if (!razdel0.empty()) {
                        if (stopWords.find(razdel0) != stopWords.end() || (ContainsDigit(razdel0))) {
                            continue;
                        }
                        razdel0 = stem(razdel0);
                        std::unordered_map<std::string, std::string> word_map;
                        word_map[razdel0] = sentence + "...";
                        data.words.push_back(word_map);
                        map_path[razdel0] = "#" + match[1].str();
                        data.path.push_back(map_path);

                        if (synonyms_map.find(razdel0) != synonyms_map.end()) {
                            for (auto synonym : synonyms_map.at(razdel0)) {
                                synonym = stem(synonym);
                                std::unordered_map<std::string, std::string> synonym_map;
                                synonym_map[synonym] = sentence + "...";
                                data.words.push_back(synonym_map);
                            }
                        }
                    }
                }
            }
        }
        search_start = match.suffix().first;
    }
    file2.close();
    file.close();

    /* Функция для вывода в txt файл "маленькие" файлы
    if (line_count <= 8) {
        std::ofstream outFile("C:\\Users\\anzim\\source\\repos\\bosst\\bosst\\files.txt", std::ios::app); // Укажите свой путь созданого txt файла
        if (outFile.is_open()) {
            outFile << file_path.string() << std::endl;
            outFile.close();
        }
        else {
            std::cerr << "Не удалось открыть файл для записи" << std::endl;
        }
    }
    */
    return data;
}

std::string ReplaceSlash(std::string output) {
    output = std::regex_replace(output, std::regex(R"(\\)"), "/");
    return output;
}

// Функция для построения индекса
WordIndex BuildWordIndex(const std::vector<fs::path>& html_files, const fs::path& docs_path, const std::unordered_map<std::string, std::vector<std::string>>& synonimus) {
    WordIndex index;

    for (const auto& file_path : html_files) {
        // Извлекаем слова и название файла
        ExtractedData data = ExtractWordsFromHtml(file_path, synonimus);

        if (data.name.empty()) {
            std::cerr << "Файл " << file_path << " не содержит тега <h6> с названием.\n";
            continue; 
        }

        // Вычисляем относительный путь от docs_path до file_path
        fs::path relative_path;
        try {
            relative_path = fs::relative(file_path, docs_path);
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "Ошибка при вычислении относительного пути для файла " << file_path
                << ": " << e.what() << std::endl;
            continue; 
        }

        fs::path final_path = ".." / relative_path;

        // Преобразуем путь в строку с использованием '/' как разделителя 
        std::string final_path_str = final_path.generic_string();

        // Создаем FileEntry с путем и названием 
        for (const auto& word_map : data.words) {
            for (const auto& [word, sentence] : word_map) {
                FileEntry entry = { final_path_str, data.name, sentence };
                for (const auto& cont : data.perehod) {
                    if (cont.count(word) > 0) {
                        entry = { final_path.parent_path().string() + "/" + ReplaceSlash(cont.at(word)), data.name, sentence };
                    }
                }
                for (const auto& map : data.path) {
                    if (map.count(word) > 0) {
                        entry = { final_path_str + map.at(word), data.name, sentence};
                    }
                }
                // Сохраняем предложение в entry 
                index[word].emplace_back(entry);
            }
        }

    }
    return index;
}


std::string escapeString(std::string input) {
    std::string output;
    for (char c : input) {
        switch (c) {
        case '\\': output += "\\\\"; break;
        case '"': output += "\\\""; break;   
        case '\'': output += "\\\'"; break;  
        case '\n': output += "\\n"; break;   
        case '\r': output += "\\r"; break;   
        case '\t': output += "\\t"; break;    
        case '<': output += "\\u003C"; break;  
        case '>': output += "\\u003E"; break; 
        case '&': output += "\\u0026"; break; 
        default: output += c; break;
        }
    }
    return output;
}

void GenerateSearchHtml(const WordIndex& index, const std::filesystem::path& output_file) {
    std::ofstream ofs(output_file, std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) {
        std::cerr << "Не удалось открыть файл для записи: " << output_file << std::endl;
        return;
    }

    ofs << "<!DOCTYPE html>\n";
    ofs << "<html lang=\"ru\">\n";
    ofs << "<head>\n";
    ofs << "    <meta charset=\"UTF-8\">\n";
    ofs << "    <title>Поисковик</title>\n";
    ofs << "    <style>\n";
    ofs << "        body { font-family: Arial, sans-serif; padding: 20px; }\n";
    ofs << "        #results { margin-top: 20px; }\n";
    ofs << "        .result-item { margin-bottom: 10px; }\n";
    ofs << "        .highlight { background-color: yellow; }\n";
    ofs << "    </style>\n";
    ofs << "</head>\n";
    ofs << "<body>\n";
    ofs << "    <h1>Поисковик</h1>\n";
    ofs << "    <input type=\"text\" id=\"searchInput\" placeholder=\"Введите слово...\" />\n";
    ofs << "    <button onclick=\"performSearch()\">Поиск</button>\n";
    ofs << "    <div id=\"results\"></div>\n";
    ofs << "    <script>\n";
    ofs << "        const index = {\n";

size_t word_count = index.size();
    size_t current_word = 0;

    for (const auto& [word, entries] : index) {
        std::unordered_map<std::string, size_t> nameCountMap;
        std::unordered_set<std::string> uniqueEntriesSet;

        // Подсчет упоминаний для каждого имени
        for (const auto& entry : entries) {
            nameCountMap[entry.name]++;
        }

        // Создание вектора для сортировки
        std::vector<std::pair<std::string, size_t>> sortedEntries(nameCountMap.begin(), nameCountMap.end());
        std::sort(sortedEntries.begin(), sortedEntries.end(), [](const auto& a, const auto& b) {
            return a.second > b.second; // Сортировка по убыванию
        });

        ofs << "            \"" << escapeString(word) << "\": [\n";

        for (const auto& [name, count] : sortedEntries) {
            for (const auto& entry : entries) {
                if (entry.name == name) {
                    std::string uniqueKey = entry.path + entry.name + entry.sentence;

                    if (uniqueEntriesSet.find(uniqueKey) == uniqueEntriesSet.end()) {
                        uniqueEntriesSet.insert(uniqueKey);
                        std::string escaped_path = escapeString(entry.path);
                        std::string escaped_name = escapeString(entry.name);
                        std::string escaped_sentence = escapeString(entry.sentence);
                        ofs << "                { \"path\": \"" << escaped_path << "\", \"name\": \"" << escaped_name << "\", \"sentence\": \"" << escaped_sentence << "\" },";
                        ofs << "\n";
                    }
                }
            }
        }

        ofs << "            ]";
        if (current_word < word_count - 1) ofs << ",";
        ofs << "\n";
        current_word++;
    }
    ofs << "        };\n\n";

    ofs << "        function escapeHtml(text) {\n";
    ofs << "            const map = {\n";
    ofs << "                '&': '&amp;',\n";
    ofs << "                '<': '&lt;',\n";
    ofs << "                '>': '&gt;',\n";
    ofs << "                '\"': '&quot;',\n";
    ofs << "                \"'\": '&#039;'\n";
    ofs << "            };\n";
    ofs << "            return text.replace(/[&<>\"']/g, function(m) { return map[m]; });\n";
    ofs << "        }\n\n";

    ofs << "        function highlightTerm(text, term) {\n";
    ofs << "            const regex = new RegExp('(' + term + ')', 'gi');\n";
    ofs << "            return text.replace(regex, '<span class=\"highlight\">\$1</span>');\n";
    ofs << "        }\n\n";

    ofs << "        function hasSuffix(word, suffix) {\n";
    ofs << "            // Проверяем, что длина слова не меньше длины суффикса\n";
    ofs << "            if (word.length >= suffix.length) {\n";
    ofs << "                // Вычисляем стартовый индекс суффикса в слове\n";
    ofs << "                const start = word.length - suffix.length;\n";
    ofs << "                // Извлекаем часть слова, начиная с рассчитанного индекса\n";
    ofs << "                const wordEnd = word.substring(start, word.length);\n";
    ofs << "                // Сравниваем извлеченную часть с суффиксом\n";
    ofs << "                return wordEnd === suffix;\n";
    ofs << "            }\n";
    ofs << "            // Если длина слова меньше длины суффикса, возвращаем false\n";
    ofs << "            return false;\n";
    ofs << "        }\n\n";


    ofs << "        function stem(word) {\n";
    ofs << "            let stemmed = word.toLowerCase();\n\n";

    ofs << "            // Список распространённых суффиксов русского языка\n";
    ofs << "            const suffixes = [\n";
    ofs << "                \"ами\", \"ями\", \"ами\", \"ями\",\n";
    ofs << "                \"уют\", \"ует\", \"уют\", \"уют\",\n";
    ofs << "                \"ают\", \"ят\",\n";
    ofs << "                \"вала\", \"вали\", \"вали\", \"вали\", \"вали\",\n";
    ofs << "                \"ивала\", \"ывала\", \"овала\", \"евала\",\n";
    ofs << "                \"ивали\", \"ывали\", \"овали\", \"евали\",\n";
    ofs << "                \"ива\", \"ыва\", \"ов\", \"ев\",\n";
    ofs << "                \"и\", \"ы\", \"а\", \"я\", \"у\", \"ю\",\n";
    ofs << "                \"ого\", \"ему\", \"ыми\", \"ими\",\n";
    ofs << "                \"ую\", \"юю\", \"ая\", \"яя\", \"ою\", \"ею\"\n";
    ofs << "            ];\n\n";

    ofs << "            for (let suffix of suffixes) {\n";
    ofs << "                if (hasSuffix(stemmed, suffix)) {\n";
    ofs << "                    stemmed = stemmed.slice(0, stemmed.length - suffix.length);\n";
    ofs << "                    break; // Останавливаемся после первого совпадения\n";
    ofs << "                }\n";
    ofs << "            }\n\n";

    ofs << "            // Дополнительные правила можно добавить здесь\n\n";

    ofs << "            return stemmed;\n";
    ofs << "        }\n\n";

    ofs << "        function performSearch() {\n";
    ofs << "            const query = document.getElementById('searchInput').value.trim().toLowerCase();\n";
      // Убираем слова, разделенные точками и нижними подчеркиваниями
    ofs << "            const cleanedQuery = query.replace(/[._\\-()]/g, ' ');\n"; // Заменяем точки и нижние подчеркивания на пустую строку
    ofs << "            const resultsDiv = document.getElementById('results');\n";
    ofs << "            resultsDiv.innerHTML = '';\n";
    ofs << "            if (cleanedQuery === '') {\n";
    ofs << "                resultsDiv.innerHTML = '<p>Введите слово для поиска.</p>';\n";
    ofs << "                return;\n";
    ofs << "            }\n";

    ofs << "            const words = cleanedQuery.split(/\\s+/).map(term => term.toLowerCase()).map(stem);\n";
    ofs << "            // Фильтруем слова, чтобы исключить слова длиной 1 или меньше\n";
    ofs << "            const filteredWords = words.filter(word => word.length > 1);\n";
    ofs << "            const resultMap = new Map();\n";

    // Поиск по полному запросу
    ofs << "            if (index.hasOwnProperty(query)) {\n";
    ofs << "                index[query].forEach(function(entry) {\n";
    ofs << "                    if (!resultMap.has(entry.path)) {\n";
    ofs << "                        resultMap.set(entry.path, { name: entry.name, sentence: entry.sentence });\n";
    ofs << "                    }\n";
    ofs << "                });\n";
    ofs << "            }\n";

    ofs << "            if (resultMap.size === 0) {\n";
    ofs << "                const firstWord = filteredWords[0];\n"; // Используем отфильтрованные слова
    ofs << "                if (index.hasOwnProperty(firstWord)) {\n";
    ofs << "                    const intermediateResults = index[firstWord];\n";
    ofs << "                    const remainingWords = filteredWords.slice(1);\n"; // Используем отфильтрованные слова
    ofs << "                    intermediateResults.forEach(function(entry) {\n";
    ofs << "                        // Проверяем, содержатся ли все оставшиеся слова в name или sentence\n";
    ofs << "                        const containsAllWordsInName = remainingWords.every(function(secondWord) {\n";
    ofs << "                            return entry.name.toLowerCase().includes(secondWord);\n";
    ofs << "                        });\n";
    ofs << "                        const containsAllWordsInSentence = remainingWords.every(function(secondWord) {\n";
    ofs << "                            return entry.sentence.toLowerCase().includes(secondWord);\n";
    ofs << "                        });\n";
    ofs << "                        if (containsAllWordsInName || containsAllWordsInSentence) {\n";
    ofs << "                            if (!resultMap.has(entry.path)) {\n";
    ofs << "                                resultMap.set(entry.path, { name: entry.name, sentence: entry.sentence });\n";
    ofs << "                            }\n";
    ofs << "                        }\n";
    ofs << "                    });\n";
    ofs << "                }\n";
    ofs << "            }\n";

    // Отображение результатов
    ofs << "            if (resultMap.size > 0) {\n";
    ofs << "                let list = '<h2>Результаты поиска для \"' + escapeHtml(query) + '\":</h2><ul>';\n";
    ofs << "                resultMap.forEach(function(entry, path) {\n";
    ofs << "                    let highlightedName = entry.name;\n";
    ofs << "                    let highlightedSentence = entry.sentence;\n";
    ofs << "                    filteredWords.forEach(function(term) {\n";
    ofs << "                        highlightedName = highlightTerm(highlightedName, escapeHtml(term));\n";
    ofs << "                        highlightedSentence = highlightTerm(highlightedSentence, escapeHtml(term));\n";
    ofs << "                    });\n";
    ofs << "                    list += '<li class=\"result-item\"><a href=\"' + escapeHtml(path) + '\" target=\"_blank\">' + highlightedName + '</a><br/><small>' + highlightedSentence + '</small></li>';\n";
    ofs << "                });\n";
    ofs << "                list += '</ul>';\n";
    ofs << "                resultsDiv.innerHTML = list;\n";
    ofs << "            } else {\n";
    ofs << "                resultsDiv.innerHTML = '<p>Ничего не найдено.</p>';\n";
    ofs << "            }\n";
    ofs << "        }\n\n";

    ofs << "        document.getElementById('searchInput').addEventListener('keypress', function(event) {\n";
    ofs << "            if (event.key === 'Enter') {\n";
    ofs << "                performSearch();\n";
    ofs << "            }\n";
    ofs << "        });\n";

    ofs << "        const clearButton = document.createElement('button');\n";
    ofs << "        clearButton.textContent = 'Очистить';\n";
    ofs << "        clearButton.onclick = function() {\n";
    ofs << "            document.getElementById('searchInput').value = '';\n";
    ofs << "            document.getElementById('results').innerHTML = '';\n";
    ofs << "        };\n";
    ofs << "        document.body.insertBefore(clearButton, document.getElementById('results'));\n";

    ofs << "    </script>\n";
    ofs << "</body>\n";
    ofs << "</html>\n";
    ofs.close();
    std::cout << "HTML-файл успешно создан: " << output_file << std::endl;
}


int main() {

    fs::path synonyms_path = "C:\\Users\\anzim\\source\\repos\\bosst\\bosst\\tm7.txt"; // Путь к файлу где записаны синонимы(напишите свой путь)

    auto synonyms_map = LoadSynonyms(synonyms_path);
    // Путь к директории с HTML-файлами
    fs::path docs_path = "C:\\Users\\anzim\\OneDrive\\Desktop\\DOCS_HTMLConverted\\rus\\"; // Путь к папке в которой содержится справка

    // Получаем список всех HTML-файлов
    std::vector<fs::path> html_files = GetAllHtmlFiles(docs_path);
    std::cout << "Найдено HTML-файлов: " << html_files.size() << std::endl;

    // Строим индекс
    WordIndex index = BuildWordIndex(html_files, docs_path, synonyms_map);
    std::cout << "Индекс построен. Уникальных слов: " << index.size() << std::endl;
    // Путь к выходному HTML-файлу
    fs::path output_html = "search_engine.html";

    // Генерируем HTML-файл
    GenerateSearchHtml(index, output_html);

    return 0;
}