#include <KAL_ESP_NOW.h>

void KAL_ESP_NOW::PrintPeers()
{
    esp_now_peer_num_t Totalpeer;
    esp_now_get_peer_num(&Totalpeer);
    Serial.printf("Memorized Peer : %d\n", Totalpeer.total_num);
    for (size_t i = 0; i < Address_List.size(); i++)
    {
        const auto &Store_Mac_Address = Address_List[i];
        Serial.print("Peer Address : ");
        Serial.print(i + 1);
        Serial.print(" : ");
        for (int j = 0; j < 6; j++)
        {
            if (Store_Mac_Address[j] < 16)
                Serial.print("0");
            Serial.print(Store_Mac_Address[j], HEX);
            if (j < 5)
                Serial.print(":");
        }
        Serial.println();
    }
}

void KAL_ESP_NOW::Setup_Esp_Now()
{
    WiFi.mode(WIFI_MODE_STA);
    WiFi.mode(WIFI_STA);
    esp_err_t result = esp_now_init();
    EspNow_Initialized = (result == ESP_OK);
    if (EspNow_Initialized == false)
    {
        WiFi.disconnect(true);
        delay(100);
        WiFi.mode(WIFI_STA);
    }
    else
    {
        Serial.println("ESP-NOW initialized successfully.");
    }
}

void KAL_ESP_NOW::Get_MAC_Address()
{
    if (esp_now_init() == ESP_OK)
    {
        Serial.print("MAC Address : ");
        Serial.println(WiFi.macAddress());
    }
    else
    {
        Serial.println("ESP-NOW init failed, MAC Address not available.");
    }
}

bool KAL_ESP_NOW::Get_Esp_Now_Init_Status()
{
    return EspNow_Initialized;
}

void KAL_ESP_NOW::Add_Peer(const uint8_t *Mac_Address)
{
    memset(&Peer_Info, 0, sizeof(Add_Peer_Info));
    memcpy(Peer_Info.peer_addr, Mac_Address, 6);
    Peer_Info.channel = 0;
    Peer_Info.encrypt = false;
    Peer_Info.ifidx = WIFI_IF_STA;

    esp_err_t result = esp_now_add_peer(&Add_Peer_Info);
    if (result == ESP_OK)
    {
        Serial.println("Peer added successfully.");
        std::array<uint8_t, 6> Store_Mac_Address;
        memcpy(Store_Mac_Address.data(), Mac_Address, 6);
        Address_List.push_back(Store_Mac_Address);
    }
    else if (result == ESP_ERR_ESPNOW_NOT_INIT)
    {
        Serial.println("ESP-NOW not initialized.");
    }
    else if (result == ESP_ERR_ESPNOW_ARG)
    {
        Serial.println("Invalid argument.");
    }
    else if (result == ESP_ERR_ESPNOW_FULL)
    {
        Serial.println("Peer list full.");
    }
    else if (result == ESP_ERR_ESPNOW_NO_MEM)
    {
        Serial.println("Out of memory.");
    }
    else if (result == ESP_ERR_ESPNOW_EXIST)
    {
        Serial.println("Peer already exists.");
    }
    else
    {
        Serial.println("Failed to add peer.");
    }
}

void KAL_ESP_NOW::Remove_Peer(const uint8_t *Mac_Address)
{

    esp_now_del_peer(Mac_Address);
    esp_err_t result = esp_now_del_peer(Mac_Address);
    if (result == ESP_OK)
    {
        Serial.println("Peer removed successfully.");
    }
    else if (result == ESP_ERR_ESPNOW_NOT_INIT)
    {
        Serial.println("ESP-NOW not initialized.");
    }
    else if (result == ESP_ERR_ESPNOW_ARG)
    {
        Serial.println("Invalid argument.");
    }
    else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
    {
        Serial.println("Peer not found.");
    }
    else
    {
        Serial.println("Failed to remove peer.");
    }
}

bool KAL_ESP_NOW::Select_Peer(const uint8_t index)
{
    const uint8_t Address_Index = Address_List.size();
    if (index < Address_Index)
    {
        memcpy(Address, Address_List[index].data(), 6);
        return true;
    }
    else
    {
        Serial.println("Invalid Peer Index.");
        return false;
    }
}

void KAL_ESP_NOW::Esp_Now_Data_Sent(size_t length, const void *data)
{
    esp_now_send(Address, (const uint8_t *)data, length);
    esp_err_t result = esp_now_send(Address, (const uint8_t *)data, length);
    if (result == ESP_OK)
    {
        Serial.println("Data sent successfully.");
    }
    else
    {
        Serial.printf("Data send failed. Error code: %d\n", result);
    }
}

bool KAL_ESP_NOW::Upon_Send_data(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    char Mac_Destination_Address[18];
    snprintf(Mac_Destination_Address, sizeof(Mac_Destination_Address), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac_addr[0], mac_addr[1],
             mac_addr[2], mac_addr[3],
             mac_addr[4], mac_addr[5]);

    if (status == ESP_NOW_SEND_SUCCESS)
    {
        Serial.printf("Data sent to %s successfully.\n", Mac_Destination_Address);
        return true;
    }
    else
    {
        Serial.printf("Data send to %s failed.\n", Mac_Destination_Address);
        return false;
    }
}

void KAL_ESP_NOW::Register_Receive_Callback()
{
    esp_now_register_recv_cb(Esp_Now_Data_Receieve);
}

void KAL_ESP_NOW::Esp_Now_Data_Receieve(const uint8_t *Mac_Address, const uint8_t *Received_Data, int length)
{
    char Mac_Destination_Address[18];
    snprintf(Mac_Destination_Address, sizeof(Mac_Destination_Address), "%02X:%02X:%02X:%02X:%02X:%02X",
             Mac_Address[0], Mac_Address[1],
             Mac_Address[2], Mac_Address[3],
             Mac_Address[4], Mac_Address[5]);

    Serial.printf("Data received from %s : ", Mac_Destination_Address);
    for (int i = 0; i < length; i++)
    {
        Serial.printf("%d ", Received_Data[i]);
    }
    Serial.println();
}
