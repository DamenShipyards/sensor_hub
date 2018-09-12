struct wdi_device_info *device, *list;

if (wdi_create_list(&list, NULL) == WDI_SUCCESS) {
    for (device = list; device != NULL; device = device->next) {
        printf("Installing driver for USB device: \"%s\" (%04X:%04X)\n",
            device->desc, device->vid, device->pid);
        if (wdi_prepare_driver(device, DEFAULT_DIR, INF_NAME, NULL) == WDI_SUCCESS) {
            wdi_install_driver(device, DEFAULT_DIR, INF_NAME, NULL);
        }
    }
    wdi_destroy_list(list);
}