// icp.cpp
// author: JJ

#include <iostream>

#include "app.cpp"

// define our application
App app;

int main()
{
    try {
        if (app.init())
            return app.run();
    }
    catch (std::exception const& e) {
        std::cerr << "App failed : " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

// TIP See CLion help at <a
// href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>.
//  Also, you can try interactive lessons for CLion by selecting
//  'Help | Learn IDE Features' from the main menu.