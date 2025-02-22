#include <GenericTarget/Network/GT_UDPService.hpp>
#include <GenericTarget/GT_GenericTarget.hpp>
using namespace gt;


// helper macro for IP comparison
#define GENERIC_TARGET_EQUAL_IP(lhs,rhs) ((lhs[0] == rhs[0]) && (lhs[1] == rhs[1]) && (lhs[2] == rhs[2]) && (lhs[3] == rhs[3]))


UDPService::UDPService(){
    postInitCompleted = false;
    configurationAssigned = false;
    latestErrorCode = 0;
}

UDPService::~UDPService(){
    Destroy();
}

bool UDPService::AssignConfiguration(UDPServiceConfiguration configuration){
    if(!configurationAssigned){
        conf = configuration;
        configurationAssigned = true;
        return true;
    }
    bool isEqual = (configuration == conf);
    if(!isEqual){
        GENERIC_TARGET_PRINT_ERROR("Ambiguous socket configuration! The desired configuration is {%s} but this socket has already been configured with {%s}!\n",configuration.ToString().c_str(),conf.ToString().c_str());
    }
    return isEqual;
}

bool UDPService::Create(void){
    const std::lock_guard<std::mutex> lock(mtxSocket);
    if(udpSocket.IsOpen()){
        GENERIC_TARGET_PRINT_ERROR("The UDP socket is already open!\n");
        return false;
    }
    if(!configurationAssigned){
        GENERIC_TARGET_PRINT_ERROR("The configuration for this UDP socket has not been set!\n");
        return false;
    }

    // create socket
    udpSocket.ResetLastError();
    if(!udpSocket.Open()){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not create UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        return false;
    }

    // socket priority
    udpSocket.ResetLastError();
    if(udpSocket.SetSocketPriority(conf.socketPriority) < 0){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not set SO_PRIORITY option for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        udpSocket.Close();
        return false;
    }

    // reuse port
    udpSocket.ResetLastError();
    if(udpSocket.ReusePort(true) < 0){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not set SO_REUSEPORT option for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        udpSocket.Close();
        return false;
    }

    // reuse address
    udpSocket.ResetLastError();
    if(udpSocket.ReuseAddrress(true) < 0){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not set SO_REUSEADDR option for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        udpSocket.Close();
        return false;
    }

    // allow broadcast
    udpSocket.ResetLastError();
    if(udpSocket.AllowBroadcast(conf.allowBroadcast) < 0){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not set SO_BROADCAST option for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        udpSocket.Close();
        return false;
    }

    // multicast all
    udpSocket.ResetLastError();
    if(udpSocket.SetMulticastAll(conf.multicastAll) < 0){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not set IP_MULTICAST_ALL option for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        udpSocket.Close();
        return false;
    }

    // multicast loop
    udpSocket.ResetLastError();
    if(udpSocket.SetMulticastLoop(conf.multicastLoop) < 0){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not set IP_MULTICAST_LOOP option for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        udpSocket.Close();
        return false;
    }

    // multicast TTL
    udpSocket.ResetLastError();
    if(udpSocket.SetMulticastTTL(conf.multicastTTL) < 0){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not set IP_MULTICAST_TTL option for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        udpSocket.Close();
        return false;
    }

    // non-blocking mode
    udpSocket.ResetLastError();
    if(!udpSocket.EnableNonBlockingMode()){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not enable non-blocking mode for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        udpSocket.Close();
        return false;
    }

    GENERIC_TARGET_PRINT("Created UDP socket (%s)\n", conf.ToString().c_str());
    return true;
}

void UDPService::Destroy(void){
    const std::lock_guard<std::mutex> lock(mtxSocket);
    if(udpSocket.IsOpen()){
        for(auto&& groupIP : currentlyJoinedGroups){
            (void) udpSocket.LeaveMulticastGroup(groupIP, conf.deviceName, conf.multicastInterfaceAddress);
        }
        udpSocket.Close();
        GENERIC_TARGET_PRINT("Destroyed UDP socket (%s)\n", conf.ToString().c_str());
    }
    currentlyJoinedGroups.clear();
    postInitCompleted = false;
    configurationAssigned = false;
    latestErrorCode = 0;
    conf.Reset();
}

std::tuple<int32_t, int32_t> UDPService::SendTo(Address destination, uint8_t *bytes, int32_t size){
    int32_t tx = -1;
    if(PostInitialization()){
        tx = 0;
        bool enableTransmission = (size > 0) || ((0 == size) && conf.allowZeroLengthMessage);
        if(enableTransmission){
            const std::lock_guard<std::mutex> lock(mtxSocket);
            udpSocket.ResetLastError();
            tx = udpSocket.SendTo(destination, bytes, size);
            latestErrorCode = udpSocket.GetLastErrorCode();
        }
    }
    return std::make_tuple(tx, latestErrorCode);
}

std::tuple<Address, int32_t, int32_t> UDPService::ReceiveFrom(uint8_t *bytes, int32_t maxSize, std::vector<std::array<uint8_t,4>> multicastGroups){
    Address source;
    int32_t rx = -1;
    if(PostInitialization()){
        UpdateMulticastGroups(multicastGroups);
        mtxSocket.lock();
        udpSocket.ResetLastError();
        rx = udpSocket.ReceiveFrom(source, bytes, maxSize);
        latestErrorCode = udpSocket.GetLastErrorCode();
        mtxSocket.unlock();
    }
    return std::make_tuple(source, rx, latestErrorCode);
}

bool UDPService::PostInitialization(void){
    if(!postInitCompleted){
        const std::lock_guard<std::mutex> lock(mtxSocket);

        // bind port
        uint16_t port = static_cast<uint16_t>(std::clamp(conf.port, 0, 65535));
        udpSocket.ResetLastError();
        if(udpSocket.Bind(port) < 0){
            auto [errorCode, errorString] = udpSocket.GetLastError();
            if(errorCode != latestErrorCode){
                GENERIC_TARGET_PRINT_WARNING("Could not bind port for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
            }
            latestErrorCode = errorCode;
            return false;
        }

        // bind to device
        udpSocket.ResetLastError();
        if(udpSocket.BindToDevice(conf.deviceName) < 0){
            auto [errorCode, errorString] = udpSocket.GetLastError();
            if(errorCode != latestErrorCode){
                GENERIC_TARGET_PRINT_WARNING("Could not bind UDP socket to a device (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
            }
            latestErrorCode = errorCode;
            return false;
        }

        // success
        postInitCompleted = true;
    }
    return postInitCompleted;
}

void UDPService::UpdateMulticastGroups(std::vector<std::array<uint8_t,4>> multicastGroups){
    // delete all entries in multicastGroups that do not indicate valid multicast addresses
    RemoveInvalidGroupAddresses(multicastGroups);
    MakeUnique(multicastGroups);
    std::vector<std::array<uint8_t,4>> joinedGroups;

    // leave groups that should be no longer joined
    for(auto&& currentlyJoinedGroup : currentlyJoinedGroups){
        bool keepJoined = false;
        for(auto&& desiredJoinedGroup : multicastGroups){
            keepJoined = GENERIC_TARGET_EQUAL_IP(currentlyJoinedGroup, desiredJoinedGroup);
            if(keepJoined){
                break;
            }
        }
        if(!keepJoined){
            const std::lock_guard<std::mutex> lock(mtxSocket);
            keepJoined = (udpSocket.LeaveMulticastGroup(currentlyJoinedGroup, conf.deviceName, conf.multicastInterfaceAddress) < 0);
        }
        if(keepJoined){
            joinedGroups.push_back(currentlyJoinedGroup);
        }
    }

    // join groups that are not currently joined
    for(auto&& desiredJoinedGroup : multicastGroups){
        bool alreadyJoined = false;
        for(auto&& currentlyJoinedGroup : currentlyJoinedGroups){
            alreadyJoined = GENERIC_TARGET_EQUAL_IP(currentlyJoinedGroup, desiredJoinedGroup);
            if(alreadyJoined){
                break;
            }
        }
        if(!alreadyJoined){
            const std::lock_guard<std::mutex> lock(mtxSocket);
            #ifdef _WIN32
            (void) udpSocket.SetMulticastInterface(desiredJoinedGroup, conf.deviceName, conf.multicastInterfaceAddress);
            #endif
            alreadyJoined = (udpSocket.JoinMulticastGroup(desiredJoinedGroup, conf.deviceName, conf.multicastInterfaceAddress) >= 0);
        }
        if(alreadyJoined){
            joinedGroups.push_back(desiredJoinedGroup);
        }
    }

    // remember unique list of joined group addresses
    MakeUnique(joinedGroups);
    currentlyJoinedGroups.swap(joinedGroups);
}

void UDPService::RemoveInvalidGroupAddresses(std::vector<std::array<uint8_t,4>>& groupAddresses) const {
    for(size_t k = 0; k < groupAddresses.size();){
        if((groupAddresses[k][0] < 224) || (groupAddresses[k][0] > 239)){
            groupAddresses.erase(groupAddresses.begin() + k);
            continue;
        }
        ++k;
    }
}

void UDPService::MakeUnique(std::vector<std::array<uint8_t,4>>& groupAddresses) const {
    for(size_t k = 0; k < groupAddresses.size();){
        bool addressAlreadyExists = false;
        for(size_t i = 0; (i < k) && !addressAlreadyExists; ++i){
            addressAlreadyExists = GENERIC_TARGET_EQUAL_IP(groupAddresses[i], groupAddresses[k]);
        }
        if(addressAlreadyExists){
            groupAddresses.erase(groupAddresses.begin() + k);
            continue;
        }
        ++k;
    }
}

