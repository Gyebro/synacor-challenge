#include "vm.h"
#include "debugger.h"

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

    //program.solve_confirmation_problem();

    debugger d;
    d.attach(&program);

    // Continue after debugging
    clear_screen();
    cout << program.get_output() << endl;

    program.add_input("north\nnorth\nnorth\nnorth\n"
                      "north\nnorth\nnorth\n"
                      "east\n"
                      "take journal\n"
                      "west\n"
                      "north\nnorth\n" // Vault Antechamber
                      "take orb\n");
    program.resume_program();
    cout << program.get_output() << endl;

    program.solve_maze_problem();

    program.resume_program(true);
    return 0;
}