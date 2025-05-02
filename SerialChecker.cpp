#include <iostream>
#include <string>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <windows.h>
#include <thread>
#include <vector>
#include <iomanip>

#pragma comment(lib, "Advapi32.lib")

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void resetColor() {
    setColor(7);
}

std::string runCommand(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) return "ERROR";
    while (fgets(buffer.data(), buffer.size(), pipe)) {
        result += buffer.data();
    }
    _pclose(pipe);
    return result;
}

std::string cleanOutput(const std::string& output) {
    std::string clean;
    for (char ch : output) {
        if (ch != '\n' && ch != '\r') clean += ch;
    }
    return clean.empty() ? "N/A" : clean;
}

void printCenteredMessage(const std::string& message) {
    const int width = 60;
    const int padding = (width - static_cast<int>(message.length())) / 2;
    std::cout << std::setw(padding + static_cast<int>(message.length())) << message << std::endl;
}

void displayHeader() {
    setColor(9);
    printCenteredMessage("BLIGHT.GG");
    resetColor();
}

bool IsAdmin() {
    BOOL isAdmin = FALSE;
    PSID AdministratorsGroup;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup)) {

        CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin);
        FreeSid(AdministratorsGroup);
    }
    return isAdmin;
}

void showAdminMessage() {
    setColor(12);
    std::cout << "Launch as Administrator!" << std::endl;
    resetColor();
    Sleep(5000);
}

void checkForTampering() {
    // Placeholder for future tampering checks
}

void saveToFile(const std::vector<std::pair<std::string, std::string>>& serials) {
    std::ofstream file("serial_output.txt");
    if (file.is_open()) {
        for (const auto& pair : serials) {
            file << pair.first.c_str() << pair.second.c_str() << "\n";
        }
        file.close();
        setColor(9);
        printCenteredMessage("Serials saved to serial_output.txt");
        resetColor();
    } else {
        setColor(12);
        printCenteredMessage("Failed to write to file.");
        resetColor();
    }
}

void displaySystemInfo(const std::vector<std::pair<std::string, std::string>>& serials) {
    setColor(9);
    printCenteredMessage("-------------------------------------------");
    setColor(15);
    printCenteredMessage("System Serial Information:");
    setColor(9);
    printCenteredMessage("-------------------------------------------");
    resetColor();

    setColor(15);
    for (const auto& pair : serials) {
        std::cout << "\n" << pair.first.c_str() << pair.second.c_str();
    }
    resetColor();
}

int main() {
    while (true) {
        if (!IsAdmin()) {
            showAdminMessage();
            return 0;
        }

        checkForTampering();

        std::string bios, baseboard, uuid, disk, cpuID, motherboard, smbios, ram, diskDrive, macAddress;

        std::thread biosThread([&]() { bios = cleanOutput(runCommand("powershell -Command \"(Get-WmiObject win32_bios).SerialNumber\"")); });
        std::thread baseboardThread([&]() { baseboard = cleanOutput(runCommand("powershell -Command \"(Get-WmiObject win32_baseboard).SerialNumber\"")); });
        std::thread uuidThread([&]() { uuid = cleanOutput(runCommand("powershell -Command \"(Get-WmiObject Win32_ComputerSystemProduct).UUID\"")); });
        std::thread diskThread([&]() { disk = cleanOutput(runCommand("powershell -Command \"(Get-WmiObject win32_physicalmedia).SerialNumber\"")); });
        std::thread cpuIDThread([&]() { cpuID = cleanOutput(runCommand("powershell -Command \"(Get-WmiObject Win32_Processor).ProcessorId\"")); });
        std::thread motherboardThread([&]() { motherboard = cleanOutput(runCommand("powershell -Command \"(Get-WmiObject Win32_BaseBoard).Product\"")); });
        std::thread smbiosThread([&]() { smbios = cleanOutput(runCommand("powershell -Command \"(Get-WmiObject Win32_SystemEnclosure).SMBIOSAssetTag\"")); });
        std::thread ramThread([&]() { ram = cleanOutput(runCommand("powershell -Command \"(Get-WmiObject Win32_PhysicalMemory).SerialNumber\"")); });
        std::thread diskDriveThread([&]() { diskDrive = cleanOutput(runCommand("powershell -Command \"(Get-WmiObject Win32_DiskDrive).SerialNumber\"")); });
        std::thread macAddressThread([&]() { macAddress = cleanOutput(runCommand("powershell -Command \"(Get-WmiObject Win32_NetworkAdapter | Where-Object { $_.MACAddress -ne $null }).MACAddress\"")); });

        biosThread.join();
        baseboardThread.join();
        uuidThread.join();
        diskThread.join();
        cpuIDThread.join();
        motherboardThread.join();
        smbiosThread.join();
        ramThread.join();
        diskDriveThread.join();
        macAddressThread.join();

        std::vector<std::pair<std::string, std::string>> serials = {
            {"[BIOS Serial Number]     ", bios},
            {"[Baseboard Serial]       ", baseboard},
            {"[System UUID]            ", uuid},
            {"[Disk Drive Serial]      ", disk},
            {"[CPU ID]                 ", cpuID},
            {"[Motherboard Model]      ", motherboard},
            {"[SMBIOS Asset Tag]       ", smbios},
            {"[RAM Serial Number]      ", ram},
            {"[Disk Drive Serial]      ", diskDrive},
            {"[MAC Serial]             ", macAddress},
        };

        displayHeader();
        displaySystemInfo(serials);

        setColor(9);
        printCenteredMessage("\nOptions:");
        printCenteredMessage("-------------------------------------------");
        printCenteredMessage("1 - Save to TXT file");
        printCenteredMessage("2 - Run Again");
        printCenteredMessage("3 - Exit");
        resetColor();
        printCenteredMessage("-------------------------------------------");
        std::cout << "\nEnter your choice: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore();

        if (choice == 1) {
            saveToFile(serials);
        } else if (choice == 2) {
            continue;
        } else if (choice == 3) {
            break;
        } else {
            setColor(12);
            printCenteredMessage("Invalid option. Try again.");
            resetColor();
        }
    }
    return 0;
}
