#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Function prototypes
void showMenu();
void memoryMenu();
void statisticsMenu();
void complexMenu();
void matrixMenu();
double computeMean(double data[], int size);
double computeStandardDeviation(double data[], int size);

// Memory to store results
double memory = 0.0;

// Structure to represent complex numbers
typedef struct {
    double real;
    double imag;
} Complex;

// Structure to represent matrices
typedef struct {
    int rows;
    int cols;
    double data[10][10];
} Matrix;

int main() {
    int choice;
    double num1, num2, result;
    char trigFunc[4];
    double data[100];
    int i, size;
    Complex c1, c2, cResult;
    Matrix m1, m2, mResult;
    char log[1000] = "";

    while (1) {
        showMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1:  // Basic Arithmetic Operations
                printf("Enter two numbers: ");
                scanf("%lf %lf", &num1, &num2);
                printf("Select an operation (+, -, *, /): ");
                char op;
                scanf(" %c", &op);
                switch (op) {
                    case '+':
                        result = num1 + num2;
                        printf("Result: %.2lf + %.2lf = %.2lf\n", num1, num2, result);
                        break;
                    case '-':
                        result = num1 - num2;
                        printf("Result: %.2lf - %.2lf = %.2lf\n", num1, num2, result);
                        break;
                    case '*':
                        result = num1 * num2;
                        printf("Result: %.2lf * %.2lf = %.2lf\n", num1, num2, result);
                        break;
                    case '/':
                        if (num2 != 0) {
                            result = num1 / num2;
                            printf("Result: %.2lf / %.2lf = %.2lf\n", num1, num2, result);
                        } else {
                            printf("Error! Division by zero.\n");
                        }
                        break;
                    default:
                        printf("Error! Invalid operation.\n");
                }
                sprintf(log + strlen(log), "%.2lf %c %.2lf = %.2lf\n", num1, op, num2, result);
                break;
            case 2:  // Trigonometric Functions
                printf("Enter function (sin, cos, tan) and angle in degrees: ");
                scanf("%s %lf", trigFunc, &num1);
                num1 = num1 * (M_PI / 180.0); // Convert degrees to radians
                if (strcmp(trigFunc, "sin") == 0) {
                    result = sin(num1);
                    printf("Result: sin(angle) = %.2lf\n", result);
                } else if (strcmp(trigFunc, "cos") == 0) {
                    result = cos(num1);
                    printf("Result: cos(angle) = %.2lf\n", result);
                } else if (strcmp(trigFunc, "tan") == 0) {
                    result = tan(num1);
                    printf("Result: tan(angle) = %.2lf\n", result);
                } else {
                    printf("Error! Invalid trigonometric function.\n");
                }
                sprintf(log + strlen(log), "%s(%.2lf) = %.2lf\n", trigFunc, num1, result);
                break;
            case 3:  // Memory Functions
                memoryMenu();
                int memChoice;
                scanf("%d", &memChoice);
                switch (memChoice) {
                    case 1:  // Store in Memory
                        memory = result;
                        printf("Stored %.2lf in memory.\n", memory);
                        break;
                    case 2:  // Recall from Memory
                        printf("Memory: %.2lf\n", memory);
                        break;
                    case 3:  // Clear Memory
                        memory = 0;
                        printf("Memory cleared.\n");
                        break;
                    default:
                        printf("Error! Invalid choice.\n");
                }
                break;
            case 4:  // Statistical Functions
                statisticsMenu();
                int statChoice;
                scanf("%d", &statChoice);
                printf("Enter number of elements: ");
                scanf("%d", &size);
                printf("Enter elements: ");
                for (i = 0; i < size; i++) {
                    scanf("%lf", &data[i]);
                }
                if (statChoice == 1) {
                    result = computeMean(data, size);
                    printf("Mean: %.2lf\n", result);
                } else if (statChoice == 2) {
                    result = computeStandardDeviation(data, size);
                    printf("Standard Deviation: %.2lf\n", result);
                } else {
                    printf("Error! Invalid choice.\n");
                }
                break;
            case 5:  // Complex Number Operations
                complexMenu();
                int complexChoice;
                scanf("%d", &complexChoice);
                printf("Enter real and imaginary parts of the first complex number: ");
                scanf("%lf %lf", &c1.real, &c1.imag);
                printf("Enter real and imaginary parts of the second complex number: ");
                scanf("%lf %lf", &c2.real, &c2.imag);
                if (complexChoice == 1) { // Addition
                    cResult.real = c1.real + c2.real;
                    cResult.imag = c1.imag + c2.imag;
                    printf("Result: %.2lf + %.2lfi\n", cResult.real, cResult.imag);
                } else if (complexChoice == 2) { // Subtraction
                    cResult.real = c1.real - c2.real;
                    cResult.imag = c1.imag - c2.imag;
                    printf("Result: %.2lf + %.2lfi\n", cResult.real, cResult.imag);
                } else if (complexChoice == 3) { // Multiplication
                    cResult.real = c1.real * c2.real - c1.imag * c2.imag;
                    cResult.imag = c1.real * c2.imag + c1.imag * c2.real;
                    printf("Result: %.2lf + %.2lfi\n", cResult.real, cResult.imag);
                } else if (complexChoice == 4) { // Division
                    double denominator = c2.real * c2.real + c2.imag * c2.imag;
                    if (denominator != 0) {
                        cResult.real = (c1.real * c2.real + c1.imag * c2.imag) / denominator;
                        cResult.imag = (c1.imag * c2.real - c1.real * c2.imag) / denominator;
                        printf("Result: %.2lf + %.2lfi\n", cResult.real, cResult.imag);
                    } else {
                        printf("Error! Division by zero.\n");
                    }
                } else {
                    printf("Error! Invalid choice.\n");
                }
                break;
            case 6:  // Matrix Operations
                matrixMenu();
                int matrixChoice;
                scanf("%d", &matrixChoice);
                printf("Enter dimensions of matrix 1 (rows cols): ");
                scanf("%d %d", &m1.rows, &m1.cols);
                printf("Enter matrix 1 elements:\n");
                for (i = 0; i < m1.rows; i++)
                    for (int j = 0; j < m1.cols; j++)
                        scanf("%lf", &m1.data[i][j]);
                printf("Enter dimensions of matrix 2 (rows cols): ");
                scanf("%d %d", &m2.rows, &m2.cols);
                printf("Enter matrix 2 elements:\n");
                for (i = 0; i < m2.rows; i++)
                    for (int j = 0; j < m2.cols; j++)
                        scanf("%lf", &m2.data[i][j]);
                // Further matrix operations can be added here
                break;
            case 0:  // Exit
                printf("Exiting the calculator. Goodbye!\n");
                printf("Calculation History:\n%s", log);
                exit(0);
            default:
                printf("Error! Invalid choice. Please try again.\n");
        }
    }
    return 0;
}

// Display the main menu
void showMenu() {
    printf("\n--- Advanced Calculator ---\n");
    printf("1. Basic Arithmetic Operations\n");
    printf("2. Trigonometric Functions\n");
    printf("3. Memory Functions\n");
    printf("4. Statistical Functions\n");
    printf("5. Complex Number Operations\n");
    printf("6. Matrix Operations\n");
    printf("0. Exit\n");
    printf("Enter your choice: ");
}

// Display the memory function menu
void memoryMenu() {
    printf("\nMemory Functions:\n");
    printf
