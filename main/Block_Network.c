#include "Block_Network.h"

void configure_ap_ip(void) {
    // 1. Получаем дескриптор интерфейса точки доступа
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    
    // 2. Останавливаем DHCP-сервер (обязательно перед изменением IP)
    esp_netif_dhcps_stop(netif);

    // 3. Заполняем структуру с новыми данными
    esp_netif_ip_info_t ip_info;
    memset(&ip_info, 0, sizeof(esp_netif_ip_info_t));
    
    // Устанавливаем желаемый IP (например, 192.168.10.1)    
    ip_info.ip.addr = esp_ip4addr_aton("192.168.10.1");
    ip_info.gw.addr = esp_ip4addr_aton("192.168.10.1");
    ip_info.netmask.addr = esp_ip4addr_aton("255.255.255.0");

    // 4. Применяем настройки
    ESP_ERROR_CHECK(esp_netif_set_ip_info(netif, &ip_info));

    // 5. Снова запускаем DHCP-сервер
    ESP_ERROR_CHECK(esp_netif_dhcps_start(netif));
}