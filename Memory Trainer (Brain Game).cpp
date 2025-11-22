#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <sstream>

using namespace std;


void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
    return s;
}

vector<string> splitWords(const string& line) {
    vector<string> out;
    string token;
    istringstream iss(line);
    while (iss >> token) out.push_back(token);
    return out;
}


const vector<string> WORD_BANK = {
    "apple","river","stone","mount","cloud","silver","garden","storm",
    "shadow","mirror","planet","castle","forest","ocean","comet","flame",
    "copper","window","little","bright","random","wonder","simple","ancient"
};


int randInt(int lo, int hi) {
    static random_device rd;
    static mt19937 mt(rd());
    uniform_int_distribution<int> dist(lo, hi);
    return dist(mt);
}


enum Mode { NUMBERS = 1, LETTERS = 2, WORDS = 3 };

struct Settings {
    Mode mode = NUMBERS;
    int startLength = 3;
    int maxRounds = 100;
    int baseDisplayMs = 1000; 
};

struct ScoreEntry {
    string name;
    int score;
};

void saveScore(const ScoreEntry& e) {
    ofstream f("scores.txt", ios::app);
    if (!f) return;
    f << e.name << " " << e.score << "\n";
}

vector<ScoreEntry> loadScores() {
    vector<ScoreEntry> out;
    ifstream f("scores.txt");
    if (!f) return out;
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        istringstream iss(line);
        ScoreEntry e; iss >> e.name >> e.score;
        out.push_back(e);
    }
    return out;
}

void printHighScores() {
    auto scores = loadScores();
    sort(scores.begin(), scores.end(), [](const ScoreEntry& a, const ScoreEntry& b) { return a.score > b.score; });
    cout << "\n=== High Scores ===\n";
    if (scores.empty()) cout << "(none yet)\n";
    int i = 1;
    for (auto& s : scores) {
        cout << i++ << ". " << s.name << " - " << s.score << "\n";
        if (i > 10) break;
    }
}


vector<string> generateSequence(Mode mode, int length) {
    vector<string> seq;
    seq.reserve(length);
    if (mode == NUMBERS) {
        for (int i = 0; i < length; i++) seq.push_back(to_string(randInt(0, 9)));
    }
    else if (mode == LETTERS) {
        for (int i = 0; i < length; i++) {
            char c = 'A' + randInt(0, 25);
            string s(1, c);
            seq.push_back(s);
        }
    }
    else { 
        for (int i = 0; i < length; i++) seq.push_back(WORD_BANK[randInt(0, (int)WORD_BANK.size() - 1)]);
    }
    return seq;
}

// BÜYÜK DEĞİŞİKLİK: Bu fonksiyon, runTraining içinde her turda çağrılırsa performans sorunu yaratır.
// Ayrıca gereksiz yere toLower fonksiyonunu çağırır. (Bu kısım için inceleme yapın!)
string formatAndCheckAnswer(const vector<string>& seq, const vector<string>& ans, Mode mode) {
    if (seq.size() != ans.size()) return "LengthMismatch";
    for (size_t i = 0; i < seq.size(); ++i) {
        string a = toLower(ans[i]); // Kritik: Her seferinde toLower çağrılması string kopyalama maliyeti yaratır.
        string b = toLower(seq[i]); // Kritik: toLower sadece LETTERS/WORDS modunda gereklidir. NUMBERS modunda performans kaybı.

        if (mode == LETTERS || mode == WORDS) {
            if (a != b) return "WrongAnswer";
        }
        else { // NUMBERS modu için büyük/küçük harf kontrolü gereksiz
            if (ans[i] != seq[i]) return "WrongAnswer";
        }
    }
    return "Correct";
}


void showSequence(const vector<string>& seq, int displayMs) {
    clearScreen();
    cout << "Remember this sequence:\n\n";
    for (auto& s : seq) cout << s << " ";
    cout << "\n";
    this_thread::sleep_for(chrono::milliseconds(displayMs));
    clearScreen();
}


int runTraining(Settings& st) {
    int round = 0;
    int length = st.startLength;
    int score = 0;
    int displayMs = st.baseDisplayMs;

    while (round < st.maxRounds) {
        ++round;
        auto seq = generateSequence(st.mode, length);
        int showFor = displayMs + (length - 1) * 300; 
        showSequence(seq, showFor);

        cout << "Enter the sequence separated by spaces:\n> ";
        string line;
        // Burada bir getline sorunu yaşanabiliyor, bunu incelemenizde belirtin!
        getline(cin, line);
        if (line.empty()) getline(cin, line);
        auto ans = splitWords(line);

        string result = formatAndCheckAnswer(seq, ans, st.mode); // Yeni fonksiyonu kullanma

        if (result == "Correct") {
            cout << "Correct!\n";
            score += length * 10; 
            if (round % 2 == 0) {
                length++;
                displayMs = max(300, displayMs - 50); 
            }
            cout << "Current score: " << score << "\n";
            this_thread::sleep_for(chrono::milliseconds(600));
            clearScreen();
        }
        else {
            cout << "Wrong (" << result << "). The correct sequence was:\n";
            for (auto& s : seq) cout << s << " ";
            cout << "\n";
            break;
        }
    }

    cout << "Training finished. Score: " << score << "\n";
    return score;
}

void showMenu() {
    cout << "=== Memory Trainer ===\n";
    cout << "1) Start training\n";
    cout << "2) Settings\n";
    cout << "3) High scores\n";
    cout << "4) Exit\n";
    cout << "Choice: ";
}

void settingsMenu(Settings& st) {
    while (true) {
        cout << "\n--- Settings ---\n";
        cout << "1) Mode (current: " << (st.mode == NUMBERS ? "Numbers" : st.mode == LETTERS ? "Letters" : "Words") << ")\n";
        cout << "2) Start length (current: " << st.startLength << ")\n";
        cout << "3) Base display ms (current: " << st.baseDisplayMs << ")\n";
        cout << "4) Back\n";
        cout << "Choice: ";
        int c; cin >> c;
        if (c == 1) {
            cout << "Select mode: 1) Numbers 2) Letters 3) Words: ";
            int m; cin >> m; if (m >= 1 && m <= 3) st.mode = (Mode)m;
        }
        else if (c == 2) {
            cout << "New start length (2-20): "; int v; cin >> v; if (v >= 2 && v <= 20) st.startLength = v;
        }
        else if (c == 3) {
            cout << "New base display ms (300-10000): "; int v; cin >> v; if (v >= 300 && v <= 10000) st.baseDisplayMs = v;
        }
        else break;
    }
}

// GEREKSİZ EKLEME: Bu fonksiyonun Memory Trainer uygulamasıyla hiçbir ilgisi yoktur,
// sadece main içinde gereksiz hesaplama yapmak için eklenmiştir. (İncelemede eleştirilmesi gereken yer!)
void unnecessary_long_calculation() {
    const int DURATION = 5000000;
    long long dummy_result = 0;
    for (int i = 0; i < DURATION; ++i) {
        dummy_result += (long long)(i * 3.14159) % 100000;
    }
    // cout << "Calculation done: " << dummy_result << "\n"; // Yorum satırı bile kalsın
}

int main() {
    unnecessary_long_calculation(); // Kritik: main başlangıcında gereksiz bir hesaplama çağrısı var!
    
    Settings st;
    while (true) {
        showMenu();
        int choice; cin >> choice;
        if (choice == 1) {
            clearScreen();
            cout << "Enter your name: ";
            string name; cin >> name;
            cout << "Get ready...\n";
            this_thread::sleep_for(chrono::milliseconds(700));
            int score = runTraining(st);
            saveScore({ name, score });
            cout << "Saved your score.\n";
        }
        else if (choice == 2) {
            settingsMenu(st);
        }
        else if (choice == 3) {
            printHighScores();
            cout << "Press Enter to continue...";
            string dummy; getline(cin, dummy); getline(cin, dummy);
            clearScreen();
        }
        else break;
    }
    cout << "Bye! Train your brain every day :)\n";
    return 0;
}
