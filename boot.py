import usb_cdc
usb_cdc.enable(console=True, data=True)
import storage
storage.remount("/", False)