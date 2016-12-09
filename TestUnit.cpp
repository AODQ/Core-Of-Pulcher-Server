#include "Console_Utility.h"
#include "TestUnit.h"
#include "Realm.h"
#include "NetObj.h"
#include "Utility.h"
#include <iostream>
#include <iomanip>
#include <map>
#include <string>
void Test_Assert(std::string test, bool value) {
  test = ' ' + test + ' ';
  std::cout << std::left << std::setw(50) << std::setfill('.') << test;
  std::cout << ' ';
  if ( value )
    Output_In_Colour("pass", CONSOLE_COLOURS::BRIGHT_GREEN);
  else
    Output_In_Colour("*FAIL*", CONSOLE_COLOURS::BRIGHT_RED);
  std::cout << '\n';
}
  
void TestUnits::Test_Units(std::vector<TestUnit> units) {
  std::cout << "--------------------- unit tests --------------------------\n";
  for ( auto unit : units ) {
    switch ( unit ) {
      case TestUnit::Networking:{
        std::cout << "---- networking\n";
        AODP::NetObj* obj[] = {
          new AODP::NetObj(0, 1, new AOD::Object()),
          new AODP::NetObj(0, 1, new AOD::Object()),
          new AODP::NetObj(0, 1, new AOD::Object()),
          new AODP::NetObj(0, 1, new AOD::Object()),
          new AODP::NetObj(0, 1, new AOD::Object())
        };
        for ( int i = 0; i != 3; ++ i ) {
          AOD::Add(obj[i]->R_AODObj());
          AODP::Net::Add_NetObj(obj[i]);
        }
        AOD::Add(obj[3]->R_AODObj());
        AOD::Add(obj[4]->R_AODObj());
        Test_Assert("new net obj is 0", obj[0]->R_NetID() == 0);
        Test_Assert("new net obj is 1", obj[1]->R_NetID() == 1);
        Test_Assert("new net obj is 2", obj[2]->R_NetID() == 2);
        Test_Assert("obj in net obj 0 is not null", obj[0]->R_AODObj());
        namespace Net = AODP::Net;
        Net::Rem_NetObj(obj[0]->R_NetID());
        std::cout << " removed net obj 0\n";
        Net::Update(Net::FORCE_UPDATE|Net::NO_NETWORK);
        Net::Add_NetObj(obj[3]);
        Test_Assert("new net obj is 0", obj[3]->R_NetID() == 0);
        AODP::Net::Add_NetObj(obj[4]);
        Test_Assert("new net obj is 3", obj[4]->R_NetID() == 3);
        Net::Rem_NetObj(obj[1]);
        Net::Rem_NetObj(obj[2]);
        Net::Rem_NetObj(obj[3]);
        Net::Rem_NetObj(obj[4]);
        Net::Update(Net::FORCE_UPDATE|Net::NO_NETWORK);
        AOD::realm->Update(AOD::Realm::UPDATE_FLAGS::UPDATE_REMOVE);
        std::cout << " removed all net objs\n";
        Test_Assert("Next netID counter is 0", Net::R_netObj_Counter() == 0);
        Test_Assert("netObjs empty", Net::R_netObj_Size() == 0);
        Test_Assert("Objects empty", AOD::realm->R_Objs_Size() == 0);
        Test_Assert("Objects to rem empty",AOD::realm->R_Objs_To_Rem() == 0);
      }break;
      case TestUnit::Utility:{
        std::cout << "---- utility\n";
        uint32_t uint32 = 310;
        uint64_t uint64 = 18446744073709551615;
        float fl = 20.2f;
        auto g = Util::Pack_Num(-4);
        auto h = Util::Pack_Num(uint32);
        auto k = Util::Pack_Num(uint64);
        auto l = Util::Pack_Num(fl);
        std::string r = "AB3@~` >";
        Test_Assert("Packing int8_t -4", -4 == Util::Unpack_Num<int8_t>(g));
        Test_Assert("Packing uint32_t " + std::to_string(uint32),
           uint32 == Util::Unpack_Num<uint32_t>(h));
        Test_Assert("Packing uint64_t " + std::to_string(uint64),
           uint64 == Util::Unpack_Num<int8_t>(Util::Pack_Num(uint64)));
        Test_Assert("Packing float " + std::to_string(fl),
           fl == Util::Unpack_Num<float>(l));
        int it = 0;
        Test_Assert("Packing string " + r,
            r == Util::Unpack_Str(Util::Pack_String(r), it));
      }break;
      case TestUnit::General:
        std::cout << "---- general\n";
        Output_In_Colour("This ",   CONSOLE_COLOURS::GREEN);
        Output_In_Colour("should ", CONSOLE_COLOURS::BLUE);
        Output_In_Colour("be ",     CONSOLE_COLOURS::BROWN);
        Output_In_Colour("in ",     CONSOLE_COLOURS::RED);
        Output_In_Colour("lots ",   CONSOLE_COLOURS::TEAL);
        Output_In_Colour("of ",     CONSOLE_COLOURS::PURPLE);
        Output_In_Colour("C",       CONSOLE_COLOURS::BRIGHT_GREEN);
        Output_In_Colour("O",       CONSOLE_COLOURS::BRIGHT_BLUE);
        Output_In_Colour("L",       CONSOLE_COLOURS::BRIGHT_BROWN);
        Output_In_Colour("O",       CONSOLE_COLOURS::BRIGHT_RED);
        Output_In_Colour("U",       CONSOLE_COLOURS::BRIGHT_TEAL);
        Output_In_Colour("R\n",     CONSOLE_COLOURS::BRIGHT_PURPLE);
      break;
    }
  }
  std::cout << "-----------------------------------------------------------\n";
}

void TestUnits::Query_Test_Units() {
  std::cout << "\n\nEnter list of test units ending with enter\n"
            << "Empty list is fine. Possible test units:\n"
            << "{ all ";
  // output possible test units
  for ( int i = 0; i != (int)TestUnits::TestUnit::Size; ++ i ) {
    std::cout << TestUnits::TestUnit_To_Str((TestUnits::TestUnit)i) << ' ';
  }
  std::cout << " }\n";
  std::vector<TestUnits::TestUnit> units;
  std::string t = "";
  // grab test units
  char c;
  do {
    c = std::cin.get();
    if ( c == ' ' || c == '\n' ) {
      auto tu = TestUnits::Str_To_TestUnit(t);
      if ( tu != TestUnits::TestUnit::Size )
        units.push_back(tu);
      if ( t == "all" ) {
        for ( int i = 0; i != (int)TestUnits::TestUnit::Size; ++ i )
          units.push_back((TestUnits::TestUnit)i);
      }
      t = "";
    } else
      t += tolower(c);
  } while ( c != '\n' );
  // perform unit tests
  if ( units.size() != 0 ) {
    TestUnits::Test_Units(units);
    std::cout << "Press enter to continue\n";
    std::cin.get();
  }
}

std::string TestUnits::TestUnit_To_Str(TestUnit tu) {
  std::map<TestUnit, std::string> tutostr = {
    { TestUnit::Utility,    "utility"    },
    { TestUnit::Networking, "networking" },
    { TestUnit::General,    "general"    },
    { TestUnit::Size,       ""           }
  };
  return tutostr[tu];
}
TestUnits::TestUnit TestUnits::Str_To_TestUnit(const std::string& str) {
  std::map<std::string, TestUnit> strtotut = {
    { "utility",    TestUnit::Utility    },
    { "networking", TestUnit::Networking },
    { "general",    TestUnit::General    }
  };
  auto k = strtotut.find(str);
  if ( k == strtotut.end() ) {
    return TestUnit::Size;
  }
  return k->second;
}
