#include <core/executable.hpp>
#include <pp_protocol/config_center_root/config_center_list.hpp>

using namespace xel;
using namespace std;

auto IC = xIoContext();
auto US = xUdpChannel();

struct xService : xUdpChannel::iListener {};

int main(int argc, char ** argv) {

    auto CL = xCommandLine(
        argc, argv,
        {
            { 'c', "config", "config", true },
        }
    );

    auto OptC = CL["config"];
    if (!OptC()) {
        cerr << "missing config file" << endl;
        return -1;
    }

    auto Lines = FileToLines(*OptC);
    if (Lines.empty()) {
        cerr << "no config center address found" << endl;
        return -1;
    }
    for (auto & L : Lines) {
        if (L.empty()) {
            cout << "empty line" << endl;
            continue;
        }
        auto A = xNetAddress::Parse(Trim(L));
        if (!A) {
            cerr << "invalid address" << endl;
            return -1;
        }
        cout << A.ToString() << endl;
    }

    return 0;
}
