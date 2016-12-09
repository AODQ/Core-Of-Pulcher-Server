#include "Console_Utility.h"
#include <string>
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#endif


void Clear_Screen() {
  #ifdef _WIN32
    DWORD count;

    // get handle, coords and amount of cells to blank
    auto h_out = GetStdHandle( STD_OUTPUT_HANDLE );
    auto coord = COORD{0, 0};
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo( h_out, &info);
    auto cell_amt = info.dwSize.X * info.dwSize.Y;

    // fill console with blank
    FillConsoleOutputCharacter(h_out, (TCHAR)' ', cell_amt, coord, &count);
    FillConsoleOutputAttribute(h_out, 0b111,      cell_amt, coord, &count);
    
    // return cursor
    SetConsoleCursorPosition( h_out, coord );
  #elif OS_LIN
    

  #endif
}

void Clear_Console_Attribute(HANDLE h) {
  SetConsoleTextAttribute( h, 0b0000111 );
}

void Output_Map_Files() {
  #ifdef _WIN32
  // get directory
  TCHAR dir[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, dir);
  
  TCHAR search_str[200];
  sprintf_s(search_str, "%s\\*.pmd", dir);

  // purty colours
  auto out_handle = GetStdHandle ( STD_OUTPUT_HANDLE );

  SetConsoleTextAttribute( out_handle, FOREGROUND_RED + BACKGROUND_BLUE );
  std::cout << "Valid files in directory:\n";
  SetConsoleTextAttribute( out_handle, FOREGROUND_RED + FOREGROUND_GREEN );

  WIN32_FIND_DATA fd;
  HANDLE h_find = ::FindFirstFile(search_str, &fd);
  if ( h_find != INVALID_HANDLE_VALUE ) {
    do {
      // check if it's tmx format
      std::cout << fd.cFileName << '\n';
    } while ( ::FindNextFile(h_find, &fd) );
    ::FindClose(h_find);
  }

  Clear_Console_Attribute( out_handle );

  #endif
}

void Output_In_Colour(std::string str, CONSOLE_COLOURS col) {
  #ifdef _WIN32
  auto out_handle = GetStdHandle ( STD_OUTPUT_HANDLE );
  switch ( col ) {
    case CONSOLE_COLOURS::GREEN:
      SetConsoleTextAttribute( out_handle, FOREGROUND_GREEN);
    break;
    case CONSOLE_COLOURS::BRIGHT_GREEN:
      SetConsoleTextAttribute( out_handle, FOREGROUND_GREEN +
                                           FOREGROUND_INTENSITY);
    break;
    case CONSOLE_COLOURS::RED:
      SetConsoleTextAttribute( out_handle, FOREGROUND_RED);
    break;
    case CONSOLE_COLOURS::BRIGHT_RED:
      SetConsoleTextAttribute( out_handle, FOREGROUND_RED +
                                           FOREGROUND_INTENSITY);
    break;
    case CONSOLE_COLOURS::BLUE:
      SetConsoleTextAttribute( out_handle, FOREGROUND_BLUE);
    break;
    case CONSOLE_COLOURS::BRIGHT_BLUE:
      SetConsoleTextAttribute( out_handle, FOREGROUND_BLUE +
                                           FOREGROUND_INTENSITY);
    break;
    case CONSOLE_COLOURS::PURPLE:
      SetConsoleTextAttribute( out_handle, FOREGROUND_BLUE +
                                           FOREGROUND_RED);
    break;
    case CONSOLE_COLOURS::BRIGHT_PURPLE:
      SetConsoleTextAttribute( out_handle, FOREGROUND_BLUE +
                       FOREGROUND_RED + FOREGROUND_INTENSITY);
    break;
    case CONSOLE_COLOURS::BROWN:
      SetConsoleTextAttribute( out_handle, FOREGROUND_RED +
                                           FOREGROUND_GREEN);
    break;
    case CONSOLE_COLOURS::BRIGHT_BROWN:
      SetConsoleTextAttribute( out_handle, FOREGROUND_RED +
                    FOREGROUND_GREEN + FOREGROUND_INTENSITY);
    break;
    case CONSOLE_COLOURS::TEAL:
      SetConsoleTextAttribute( out_handle, FOREGROUND_GREEN +
                                           FOREGROUND_BLUE);
    break;
    case CONSOLE_COLOURS::BRIGHT_TEAL:
      SetConsoleTextAttribute( out_handle, FOREGROUND_BLUE +
                    FOREGROUND_GREEN + FOREGROUND_INTENSITY);
    break;
  }
  #endif
  std::cout << str;
  #ifdef _WIN32
  Clear_Console_Attribute( out_handle );
  #endif
}