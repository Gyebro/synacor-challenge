#include "vm.h"

int main() {

    vm program("challenge.bin");

    // Add input to avoid typing
    program.add_input("take tablet\n"
              "use tablet\n" /* Code 4 */
              "doorway\n"
              "north\n"
              "north\n"
              "bridge\n"
              "continue\n"
              "down\n"
              "east\n"
              "take empty lantern\n"
              "west\n"
              "west\n"
              "passage\n"
              "ladder\n"
              "west\n"
              "south\n"
              "north\n" /* Code 5 */
              "take can\n"
              "use can\n"
              "use lantern\n"
              "west\n"
              "ladder\n"
              "darkness\n"
              "continue\n"
              "west\nwest\nwest\nwest\n"
              "north\n"
              "take red coin\n"
              "north\n"
              "west\n"
              "take blue coin\n"
              "up\n"
              "take shiny coin\n"
              "down\neast\neast\n"
              "take concave coin\n"
              "down\n"
              "take corroded coin\n"
              "up\nwest\n"
    );

    program.solve_coin_problem();

    program.add_input("north\n"
              "take teleporter\n"
              "use teleporter\n" /* Code 6 */
              "take business card\n"
              "take strange book\n"
    );

    // Run the program
    program.run_program(0);

    // Enter debug loop
    bool debugging = true;
    while (debugging) {
        cout << "VM Debugger:\n";
        cout << "r: print register values\n";
        cout << "r N: print register values\n";
        cout << "s: set register value\n";
        cout << "o: view last output\n";
        cout << "i input: send input to program\n";
        cout << "q: quit debugging\n";
        string line;
        getline(cin, line);
        cout << "Command: " << line << endl;
        switch (line[0]) {
            case 'o':
                cout << program.get_output() << endl;
                break;
            case 'r':
                cout << program.print_registers() << endl;
                break;
            case 'q':
                debugging = false;
                break;
        }
        if (line == "o") {

        }
    }



    getchar();

    return 0;
}