#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <ctime>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <cctype>

using namespace std;

// ---------- Enumerations ----------
enum class Genre { Fiction, NonFiction, Mystery, Science, Biography };
enum class BookStatus { Available, CheckedOut };

// ---------- Helper functions ----------
string trim(const string& text) {
    const string whitespace = " \t\r\n";
    size_t first = text.find_first_not_of(whitespace);
    if (first == string::npos) return "";
    size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

string toLower(string text) {
    transform(text.begin(), text.end(), text.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return text;
}

string genreToString(Genre genre) {
    switch (genre) {
        case Genre::Fiction: return "Fiction";
        case Genre::NonFiction: return "NonFiction";
        case Genre::Mystery: return "Mystery";
        case Genre::Science: return "Science";
        case Genre::Biography: return "Biography";
    }
    return "Fiction";
}

Genre stringToGenre(string text) {
    text = toLower(trim(text));
    if (text == "fiction") return Genre::Fiction;
    if (text == "nonfiction" || text == "non-fiction" || text == "non fiction") return Genre::NonFiction;
    if (text == "mystery") return Genre::Mystery;
    if (text == "science") return Genre::Science;
    if (text == "biography") return Genre::Biography;
    throw invalid_argument("Unknown genre. Use Fiction, NonFiction, Mystery, Science, or Biography.");
}

string statusToString(BookStatus status) {
    return status == BookStatus::Available ? "Available" : "CheckedOut";
}

// ---------- Date class ----------
class Date {
private:
    int year, month, day;

public:
    Date() : year(1970), month(1), day(1) {}

    Date(int y, int m, int d) : year(y), month(m), day(d) {}

    static Date today() {
        time_t now = time(nullptr);
        tm* local = localtime(&now);
        if (!local) throw runtime_error("Could not read the current date.");
        return Date(local->tm_year + 1900, local->tm_mon + 1, local->tm_mday);
    }

    Date addDays(int days) const {
        tm value = {};
        value.tm_year = year - 1900;
        value.tm_mon = month - 1;
        value.tm_mday = day + days;
        value.tm_hour = 12;
        mktime(&value); // Normalizes month/year boundaries.
        return Date(value.tm_year + 1900, value.tm_mon + 1, value.tm_mday);
    }

    string toString() const {
        ostringstream out;
        out << setfill('0') << setw(4) << year << '-'
            << setw(2) << month << '-' << setw(2) << day;
        return out.str();
    }
};

// ---------- Base Book class ----------
class Book {
protected:
    string title;
    string author;
    Genre genre;
    BookStatus status;

public:
    Book(string t, string a, Genre g)
        : title(trim(t)), author(trim(a)), genre(g), status(BookStatus::Available) {
        if (title.empty() || author.empty()) {
            throw invalid_argument("Book title and author cannot be empty.");
        }
    }

    virtual ~Book() = default;

    virtual void displayInfo() const {
        cout << *this << " | Type: Book" << '\n';
    }

    virtual string getType() const { return "Book"; }
    virtual string getExtraData() const { return ""; }

    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    Genre getGenre() const { return genre; }
    BookStatus getStatus() const { return status; }
    void setStatus(BookStatus s) { status = s; }

    bool operator==(const Book& other) const {
        return toLower(title) == toLower(other.title) &&
               toLower(author) == toLower(other.author);
    }

    friend ostream& operator<<(ostream& os, const Book& b) {
        os << '"' << b.title << "\" by " << b.author
           << " | Genre: " << genreToString(b.genre)
           << " | Status: " << statusToString(b.status);
        return os;
    }
};

// ---------- Derived book classes ----------
class EBook : public Book {
private:
    double fileSizeMB;

public:
    EBook(string t, string a, Genre g, double size)
        : Book(t, a, g), fileSizeMB(size) {
        if (size <= 0) throw invalid_argument("EBook file size must be greater than zero.");
    }

    void displayInfo() const override {
        cout << *this << " | Type: EBook | File size: " << fileSizeMB << " MB" << '\n';
    }

    string getType() const override { return "EBook"; }

    string getExtraData() const override {
        ostringstream out;
        out << fileSizeMB;
        return out.str();
    }
};

class PrintedBook : public Book {
private:
    int pageCount;

public:
    PrintedBook(string t, string a, Genre g, int pages)
        : Book(t, a, g), pageCount(pages) {
        if (pages <= 0) throw invalid_argument("Printed book page count must be greater than zero.");
    }

    void displayInfo() const override {
        cout << *this << " | Type: PrintedBook | Pages: " << pageCount << '\n';
    }

    string getType() const override { return "PrintedBook"; }

    string getExtraData() const override { return to_string(pageCount); }
};

// ---------- Patron class ----------
class Patron {
private:
    string name;
    int id;
    vector<string> borrowedBookTitles;

public:
    Patron(string n = "", int i = 0) : name(trim(n)), id(i) {
        if (name.empty()) throw invalid_argument("Patron name cannot be empty.");
        if (id <= 0) throw invalid_argument("Patron ID must be a positive integer.");
    }

    string getName() const { return name; }
    int getId() const { return id; }
    const vector<string>& getBorrowedBooks() const { return borrowedBookTitles; }

    void borrowBook(const string& title) {
        borrowedBookTitles.push_back(title);
    }

    void returnBook(const string& title) {
        auto it = find_if(borrowedBookTitles.begin(), borrowedBookTitles.end(),
            [&](const string& existing) { return toLower(existing) == toLower(title); });
        if (it == borrowedBookTitles.end()) {
            throw runtime_error("This patron has not borrowed that book.");
        }
        borrowedBookTitles.erase(it);
    }

    bool hasBorrowed(const string& title) const {
        return any_of(borrowedBookTitles.begin(), borrowedBookTitles.end(),
            [&](const string& existing) { return toLower(existing) == toLower(title); });
    }

    void displayPatron() const {
        cout << *this << '\n';
        cout << "   Borrowed books: ";
        if (borrowedBookTitles.empty()) {
            cout << "None";
        } else {
            for (size_t i = 0; i < borrowedBookTitles.size(); ++i) {
                if (i) cout << ", ";
                cout << borrowedBookTitles[i];
            }
        }
        cout << '\n';
    }

    bool operator==(const Patron& other) const { return id == other.id; }

    friend ostream& operator<<(ostream& os, const Patron& p) {
        os << "Patron ID: " << p.id << " | Name: " << p.name;
        return os;
    }
};

// ---------- Transaction class ----------
class Transaction {
private:
    int patronId;
    string patronName;
    string bookTitle;
    string action;
    string date;
    string dueDate;

public:
    Transaction(int pid, string pname, string bt, string act, string transactionDate, string due = "")
        : patronId(pid), patronName(move(pname)), bookTitle(move(bt)),
          action(move(act)), date(move(transactionDate)), dueDate(move(due)) {}

    void displayTransaction() const {
        cout << date << " | " << action << " | Patron " << patronId
             << " (" << patronName << ") | Book: " << bookTitle;
        if (!dueDate.empty()) cout << " | Due: " << dueDate;
        cout << '\n';
    }

    string toFileLine() const {
        return to_string(patronId) + "," + patronName + "," + bookTitle + "," +
               action + "," + date + "," + dueDate;
    }
};

// ---------- Generic file save/load templates ----------
template <typename T, typename Writer>
void saveCollection(const string& filename, const T& collection, Writer writer) {
    ofstream file(filename);
    if (!file) throw runtime_error("Could not open " + filename + " for writing.");
    for (const auto& item : collection) writer(file, item);
    if (!file) throw runtime_error("Failed while writing to " + filename + ".");
}

template <typename Reader>
void loadLines(const string& filename, Reader reader) {
    ifstream file(filename);
    if (!file) throw runtime_error("Could not open " + filename + " for reading.");
    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (!line.empty()) reader(line);
    }
}

// ---------- Library class ----------
class Library {
private:
    vector<unique_ptr<Book>> books;
    map<int, Patron> patrons;
    vector<Transaction> transactions;

    Book* findBook(const string& title) {
        for (auto& book : books) {
            if (toLower(book->getTitle()) == toLower(trim(title))) return book.get();
        }
        return nullptr;
    }

    const Book* findBook(const string& title) const {
        for (const auto& book : books) {
            if (toLower(book->getTitle()) == toLower(trim(title))) return book.get();
        }
        return nullptr;
    }

    Patron& findPatron(int id) {
        auto it = patrons.find(id);
        if (it == patrons.end()) throw runtime_error("No patron found with ID " + to_string(id) + ".");
        return it->second;
    }

    static vector<string> splitCSV(const string& line) {
        vector<string> fields;
        string field;
        stringstream ss(line);
        while (getline(ss, field, ',')) fields.push_back(trim(field));
        return fields;
    }

    void logTransaction(const Patron& patron, const string& title,
                        const string& action, const string& due = "") {
        transactions.emplace_back(patron.getId(), patron.getName(), title, action,
                                  Date::today().toString(), due);
    }

public:
    Library() = default;
    ~Library() = default; // unique_ptr automatically deletes dynamically allocated books.

    void addBook(unique_ptr<Book> book) {
        if (!book) throw invalid_argument("Cannot add an empty book object.");
        if (findBook(book->getTitle())) throw runtime_error("A book with that title already exists.");
        books.push_back(move(book));
    }

    void addPatron(const Patron& patron) {
        if (patrons.count(patron.getId())) throw runtime_error("That patron ID already exists.");
        patrons.emplace(patron.getId(), patron);
    }

    void loadData() {
        // Seed files on first run, so the program is ready to use immediately.
        ifstream checkBooks("books.txt");
        if (!checkBooks.good()) {
            ofstream seed("books.txt");
            if (!seed) throw runtime_error("Could not create books.txt.");
            seed << "Fiction,The Great Gatsby,F. Scott Fitzgerald,PrintedBook,180,Available\n"
                 << "Science,Quantum Physics 101,Albert Einstein,EBook,4.5,Available\n"
                 << "Mystery,The Silent Patient,Alex Michaelides,PrintedBook,336,Available\n"
                 << "Biography,Long Walk to Freedom,Nelson Mandela,PrintedBook,656,Available\n"
                 << "NonFiction,The Power of Habit,Charles Duhigg,EBook,8.2,Available\n";
        }

        ifstream checkPatrons("patrons.txt");
        if (!checkPatrons.good()) {
            ofstream seed("patrons.txt");
            if (!seed) throw runtime_error("Could not create patrons.txt.");
            seed << "1,Jane Doe\n2,John Smith\n3,Alex Johnson\n";
        }

        // Existing in-memory data is cleared before loading.
        books.clear();
        patrons.clear();

        loadLines("books.txt", [&](const string& line) {
            auto f = splitCSV(line);
            if (f.size() < 5) throw runtime_error("Invalid row in books.txt: " + line);
            Genre g = stringToGenre(f[0]);
            unique_ptr<Book> book;
            if (toLower(f[3]) == "ebook") {
                book = make_unique<EBook>(f[1], f[2], g, stod(f[4]));
            } else if (toLower(f[3]) == "printed" || toLower(f[3]) == "printedbook") {
                book = make_unique<PrintedBook>(f[1], f[2], g, stoi(f[4]));
            } else {
                throw runtime_error("Unknown book type in books.txt: " + f[3]);
            }
            if (f.size() >= 6 && toLower(f[5]) == "checkedout") book->setStatus(BookStatus::CheckedOut);
            addBook(move(book));
        });

        loadLines("patrons.txt", [&](const string& line) {
            auto f = splitCSV(line);
            if (f.size() < 2) throw runtime_error("Invalid row in patrons.txt: " + line);
            addPatron(Patron(f[1], stoi(f[0])));
        });

        // Reconstruct patron borrowed-book lists from checked-out book statuses.
        // This keeps return validation consistent after a restart.
        ifstream transactionFile("transactions.txt");
        if (transactionFile) {
            string line;
            while (getline(transactionFile, line)) {
                auto f = splitCSV(line);
                if (f.size() < 6) continue;
                int id = stoi(f[0]);
                transactions.emplace_back(id, f[1], f[2], f[3], f[4], f[5]);
                auto p = patrons.find(id);
                Book* book = findBook(f[2]);
                if (p == patrons.end() || !book) continue;
                if (toLower(f[3]) == "checkout") {
                    if (!p->second.hasBorrowed(f[2])) p->second.borrowBook(f[2]);
                    book->setStatus(BookStatus::CheckedOut);
                } else if (toLower(f[3]) == "return") {
                    if (p->second.hasBorrowed(f[2])) p->second.returnBook(f[2]);
                    book->setStatus(BookStatus::Available);
                }
            }
        }
    }

    void saveData() const {
        saveCollection("books.txt", books, [](ofstream& file, const unique_ptr<Book>& book) {
            file << genreToString(book->getGenre()) << ','
                 << book->getTitle() << ',' << book->getAuthor() << ','
                 << book->getType() << ',' << book->getExtraData() << ','
                 << statusToString(book->getStatus()) << '\n';
        });

        saveCollection("patrons.txt", patrons, [](ofstream& file, const pair<const int, Patron>& entry) {
            file << entry.first << ',' << entry.second.getName() << '\n';
        });

        saveCollection("transactions.txt", transactions, [](ofstream& file, const Transaction& transaction) {
            file << transaction.toFileLine() << '\n';
        });
    }

    void addBookFromInput() {
        cout << "Book type (1 = Printed, 2 = EBook): ";
        int type = readInt();
        string title, author, genreText;
        cout << "Title: ";
        getline(cin >> ws, title);
        cout << "Author: ";
        getline(cin, author);
        cout << "Genre (Fiction/NonFiction/Mystery/Science/Biography): ";
        getline(cin, genreText);
        Genre genre = stringToGenre(genreText);

        if (type == 1) {
            cout << "Page count: ";
            int pages = readInt();
            addBook(make_unique<PrintedBook>(title, author, genre, pages));
        } else if (type == 2) {
            cout << "File size in MB: ";
            double size = readDouble();
            addBook(make_unique<EBook>(title, author, genre, size));
        } else {
            throw invalid_argument("Book type must be 1 or 2.");
        }
        cout << "Book added successfully.\n";
    }

    void addPatronFromInput() {
        cout << "New patron ID (positive integer): ";
        int id = readInt();
        string name;
        cout << "Patron name: ";
        getline(cin >> ws, name);
        addPatron(Patron(name, id));
        cout << "Patron added successfully.\n";
    }

    static int readInt() {
        int value;
        if (!(cin >> value)) {
            cin.clear();
            cin.ignore(10000, '\n');
            throw invalid_argument("Please enter a valid whole number.");
        }
        return value;
    }

    static double readDouble() {
        double value;
        if (!(cin >> value) || value <= 0) {
            cin.clear();
            cin.ignore(10000, '\n');
            throw invalid_argument("Please enter a number greater than zero.");
        }
        return value;
    }

    void checkoutBook(int patronId, const string& title) {
        Patron& patron = findPatron(patronId);
        Book* book = findBook(title);
        if (!book) throw runtime_error("Book not found: " + title);
        if (book->getStatus() == BookStatus::CheckedOut) throw runtime_error("That book is already checked out.");
        if (patron.hasBorrowed(title)) throw runtime_error("This patron already has that book checked out.");

        Date due = Date::today().addDays(14);
        book->setStatus(BookStatus::CheckedOut);
        patron.borrowBook(book->getTitle());
        logTransaction(patron, book->getTitle(), "Checkout", due.toString());
        cout << "Checkout successful. Due date: " << due.toString() << '\n';
    }

    void returnBook(int patronId, const string& title) {
        Patron& patron = findPatron(patronId);
        Book* book = findBook(title);
        if (!book) throw runtime_error("Book not found: " + title);
        if (book->getStatus() == BookStatus::Available) throw runtime_error("That book is not checked out.");
        if (!patron.hasBorrowed(title)) throw runtime_error("That patron did not check out this book.");

        patron.returnBook(title);
        book->setStatus(BookStatus::Available);
        logTransaction(patron, book->getTitle(), "Return");
        cout << "Return successful.\n";
    }

    void displayBooks() const {
        if (books.empty()) { cout << "No books in the library.\n"; return; }
        cout << "\n--- Books ---\n";
        for (const auto& book : books) {
            book->displayInfo(); // Runtime polymorphism calls the derived version.
        }
    }

    void displayPatrons() const {
        if (patrons.empty()) { cout << "No patrons registered.\n"; return; }
        cout << "\n--- Patrons ---\n";
        for (const auto& entry : patrons) entry.second.displayPatron();
    }

    void searchByAuthor(const string& query) const {
        bool found = false;
        for (const auto& book : books) {
            if (toLower(book->getAuthor()).find(toLower(query)) != string::npos) {
                book->displayInfo();
                found = true;
            }
        }
        if (!found) cout << "No books found for that author search.\n";
    }

    void searchByGenre(Genre genre) const {
        bool found = false;
        for (const auto& book : books) {
            if (book->getGenre() == genre) {
                book->displayInfo();
                found = true;
            }
        }
        if (!found) cout << "No books found in that genre.\n";
    }

    void displayTransactions() const {
        cout << "\n--- Transactions recorded this session ---\n";
        if (transactions.empty()) { cout << "No transactions recorded in this session.\n"; return; }
        for (const auto& transaction : transactions) transaction.displayTransaction();
    }

    void runMenu() {
        bool running = true;
        while (running) {
            cout << "\n========== LIBRARY MANAGEMENT SYSTEM ==========\n"
                 << "1. Display all books\n"
                 << "2. Display all patrons\n"
                 << "3. Check out a book\n"
                 << "4. Return a book\n"
                 << "5. Add a book\n"
                 << "6. Add a patron\n"
                 << "7. Search books by author\n"
                 << "8. Search books by genre\n"
                 << "9. Display transactions this session\n"
                 << "10. Save data\n"
                 << "0. Save and exit\n"
                 << "Choose an option: ";

            try {
                int choice = readInt();
                switch (choice) {
                    case 1: displayBooks(); break;
                    case 2: displayPatrons(); break;
                    case 3: {
                        cout << "Patron ID: ";
                        int id = readInt();
                        string title;
                        cout << "Book title: ";
                        getline(cin >> ws, title);
                        checkoutBook(id, title);
                        break;
                    }
                    case 4: {
                        cout << "Patron ID: ";
                        int id = readInt();
                        string title;
                        cout << "Book title: ";
                        getline(cin >> ws, title);
                        returnBook(id, title);
                        break;
                    }
                    case 5: addBookFromInput(); break;
                    case 6: addPatronFromInput(); break;
                    case 7: {
                        string author;
                        cout << "Enter all or part of the author's name: ";
                        getline(cin >> ws, author);
                        searchByAuthor(author);
                        break;
                    }
                    case 8: {
                        string genre;
                        cout << "Genre (Fiction/NonFiction/Mystery/Science/Biography): ";
                        getline(cin >> ws, genre);
                        searchByGenre(stringToGenre(genre));
                        break;
                    }
                    case 9: displayTransactions(); break;
                    case 10:
                        saveData();
                        cout << "Data saved successfully.\n";
                        break;
                    case 0:
                        saveData();
                        cout << "Data saved. Goodbye!\n";
                        running = false;
                        break;
                    default:
                        cout << "Please choose a menu option from 0 to 10.\n";
                }
            } catch (const exception& e) {
                cout << "Error: " << e.what() << '\n';
            }
        }
    }
};

int main() {
    try {
        Library library;
        library.loadData();
        cout << "Library data loaded successfully.\n";
        library.runMenu();
    } catch (const exception& e) {
        cerr << "Application error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
