#include "./cdrflex_bindings.hpp"
#include <string>

void bOpenConnection(py::class_<DRAFramework::CDRFLEx>& c) {
    c.def("open_connection",
        [](DRAFramework::CDRFLEx& self, string ipAddr, unsigned int port){
            return self.open_connection(ipAddr, port);
        },
        py::arg("ip") = "192.168.137.100",
        py::arg("port") = 12345,
        "Connect to the Doosan robot controller");
}

void bCloseConnection(py::class_<DRAFramework::CDRFLEx>& c) {
    c.def("close_connection",
    [](DRAFramework::CDRFLEx& self) {
        return self.close_connection();
    },
    "Close connection to the Doosan robot controller");
}
