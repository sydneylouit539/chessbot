#include <iostream>  // Include the iostream library for input and output

// Main function where the program starts execution
int main() {
    // Print "Hello, World!" to the console
    std::cout << "Hello, World!" << std::endl;

    // Declare variables
    int number1, number2, sum;

    // Prompt user for input
    std::cout << "Enter two integers: ";
    std::cin >> number1 >> number2;

    // Calculate the sum of the two numbers
    sum = number1 + number2;

    // Print the result
    std::cout << "The sum of " << number1 << " and " << number2 << " is " << sum << std::endl;

    // Return 0 to indicate successful execution
    return 0;
}
