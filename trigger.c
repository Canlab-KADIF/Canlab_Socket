#include "util.h"

void display_banner(const char* message) {
    printf("\n+--------------------------------------------+\n");
    printf("|                                            |\n");
    printf("|    %-40s|\n", message);
    printf("|                                            |\n");
    printf("|    Version: %-30s |\n", VERSION);
    printf("|                                            |\n");
    printf("+--------------------------------------------+\n");
}

void handle_trigger(const char* message) {
    const char* triggers[] = {"control", "perception", "localization", "planning", "대분류_중분류_소분류_날씨_시간_코멘트", "1_2_3_4_5_6"};
    size_t num_triggers = sizeof(triggers) / sizeof(triggers[0]);

    for (size_t i = 0; i < num_triggers; i++) {
        if (strstr(message, triggers[i]) != NULL) {
            extern TRIGGER_CLIENT T;
            T.Trigger = 1;
            log_message("Receive Trigger :", triggers[i]);
            break;
        }
    }
}

