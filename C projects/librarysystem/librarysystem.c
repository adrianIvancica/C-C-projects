#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TITLE_LENGTH 100
#define MAX_AUTHOR_LENGTH 100
#define MAX_BOOKS 1000

// storage
typedef struct {
    int id;
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    int year;
} Book;

void addBook();
void viewBooks();
void searchBooks();
void deleteBook();
void saveBooks();
void loadBooks();

// Global array to store books and a counter for the number of books
Book library[MAX_BOOKS];
int bookCount = 0;

// File name to save and load books
const char *fileName = "library.dat";

int main() {
    int choice;

    // Load books from file when the program starts
    loadBooks();

    while (1) {
        printf("\n--- Library Management System ---\n");
        printf("1. Add a New Book\n");
        printf("2. View All Books currently stored\n");
        printf("3. Search Books from system\n");
        printf("4. Delete a Book from system\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                addBook();
                break;
            case 2:
                viewBooks();
                break;
            case 3:
                searchBooks();
                break;
            case 4:
                deleteBook();
                break;
            case 5:
                saveBooks();
                printf("Exiting the program. Goodbye!\n");
                exit(0);
                break;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    }

    return 0;
}

// Function to add a new book
void addBook() {
    if (bookCount >= MAX_BOOKS) {
        printf("Library is full! Cannot add more books.\n");
        return;
    }

    Book newBook;
    newBook.id = bookCount + 1;  // Auto-increment ID

    printf("Enter book title: ");
    getchar();  // Clear the newline character from input buffer
    fgets(newBook.title, MAX_TITLE_LENGTH, stdin);
    newBook.title[strcspn(newBook.title, "\n")] = '\0';  // Remove newline character

    printf("Enter author name: ");
    fgets(newBook.author, MAX_AUTHOR_LENGTH, stdin);
    newBook.author[strcspn(newBook.author, "\n")] = '\0'; 

    printf("Enter publication year: ");
    scanf("%d", &newBook.year);

    library[bookCount] = newBook;
    bookCount++;

    printf("Book added successfully!\n");
}

// Function to view all books
void viewBooks() {
    if (bookCount == 0) {
        printf("No books in the library.\n");
        return;
    }

    printf("\n--- List of Books ---\n");
    for (int i = 0; i < bookCount; i++) {
        printf("ID: %d\n", library[i].id);
        printf("Title: %s\n", library[i].title);
        printf("Author: %s\n", library[i].author);
        printf("Year: %d\n", library[i].year);
        printf("----------------------------\n");
    }
}

// Function to search books by title or author
void searchBooks() {
    char keyword[MAX_TITLE_LENGTH];
    int found = 0;

    printf("Enter a keyword to search the system (title or author): ");
    getchar();  // Clear the newline character from input buffer
    fgets(keyword, MAX_TITLE_LENGTH, stdin);
    keyword[strcspn(keyword, "\n")] = '\0';  // Remove newline character

    printf("\n--- Search Results ---\n");
    for (int i = 0; i < bookCount; i++) {
        if (strstr(library[i].title, keyword) != NULL || strstr(library[i].author, keyword) != NULL) {
            printf("ID: %d\n", library[i].id);
            printf("Title: %s\n", library[i].title);
            printf("Author: %s\n", library[i].author);
            printf("Year: %d\n", library[i].year);
            printf("----------------------------\n");
            found = 1;
        }
    }

    if (!found) {
        printf("No books found with the given keyword.\n");
    }
}

// Function to delete a book by ID
void deleteBook() {
    int id, found = 0;

    printf("Enter the ID of the book to delete from system: ");
    scanf("%d", &id);

    for (int i = 0; i < bookCount; i++) {
        if (library[i].id == id) {
            // Shift all books after the deleted one to the left
            for (int j = i; j < bookCount - 1; j++) {
                library[j] = library[j + 1];
            }
            bookCount--;
            found = 1;
            printf("Book deleted successfully!\n");
            break;
        }
    }

    if (!found) {
        printf("Book not found!\n");
    }
}

// Function to save books to a file
void saveBooks() {
    FILE *file = fopen(fileName, "wb");
    if (file == NULL) {
        printf("Error opening file for saving!\n");
        return;
    }

    fwrite(&bookCount, sizeof(int), 1, file);
    fwrite(library, sizeof(Book), bookCount, file);

    fclose(file);
    printf("Books saved successfully!\n");
}

// Function to load books from a file
void loadBooks() {
    FILE *file = fopen(fileName, "rb");
    if (file == NULL) {
        printf("No existing library data found. Starting with an empty library.\n");
        return;
    }

    fread(&bookCount, sizeof(int), 1, file);
    fread(library, sizeof(Book), bookCount, file);

    fclose(file);
    printf("Loaded %d books from the library.\n", bookCount);
}
