#include <usb.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>

#define VENDOR_ID  0x0c45
#define PRODUCT_ID 0x7401

#define INTERFACE1 0x00
#define INTERFACE2 0x01

const static int reqIntLen=8;
const static int endpoint_Int_in=0x82; /* endpoint 0x81 address for IN */
const static int endpoint_Int_out=0x00; /* endpoint 1 address for OUT */
const static int timeout=5000; /* timeout in ms */

const static char uTemp[] = { 0x01, 0x80, 0x33, 0x01, 0x00, 0x00, 0x00, 0x00 };
const static char uIni1[] = { 0x01, 0x82, 0x77, 0x01, 0x00, 0x00, 0x00, 0x00 };
const static char uIni2[] = { 0x01, 0x86, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00 };

/* Offset of temperature in read buffer; varies by product */
static size_t tempOffset;

static int debug=0;

/* Even within the same VENDOR_ID / PRODUCT_ID, there are hardware variations
 * which we can detect by reading the USB product ID string. This determines
 * where the temperature offset is stored in the USB read buffer. */
const static struct product_hw {
  size_t      offset;
  const char *id_string;
} product_ids[] = {
  { 4, "TEMPer1F_V1.3" },
  { 2, 0 } /* default offset is 2 */
};

void bad(const char *why) {
  fprintf(stderr,"Fatal error> %s\n",why);
  exit(17);
}


usb_dev_handle *find_lvr_winusb();

void usb_detach(usb_dev_handle *lvr_winusb, int iInterface) {
  int ret;

  ret = usb_detach_kernel_driver_np(lvr_winusb, iInterface);
  if (ret) {
    if (errno == ENODATA) {
      if (debug) {
        printf("Device already detached\n");
      }
    }
    else {
      if (debug) {
        printf("Detach failed: %s[%d]\n", strerror(errno), errno);
        printf("Continuing anyway\n");
      }
    }
  }
  else {
    if (debug) {
      printf("detach successful\n");
    }
  }
}

usb_dev_handle* setup_libusb_access() {
  usb_dev_handle *lvr_winusb;

  if (debug) {
    usb_set_debug(255);
  }
  else {
    usb_set_debug(0);
  }
  usb_init();
  usb_find_busses();
  usb_find_devices();


  if (!(lvr_winusb = find_lvr_winusb())) {
    fprintf(stderr, "Couldn't find the USB device, exiting.\n");
    return NULL;
  }

  usb_detach(lvr_winusb, INTERFACE1);
  usb_detach(lvr_winusb, INTERFACE2);

  // reset device
  if (usb_reset(lvr_winusb) != 0) {
    fprintf(stderr, "Could not reset device.\n");
    return NULL;
  }

  if (usb_set_configuration(lvr_winusb, 0x01) < 0) {
    fprintf(stderr, "Could not set configuration 1.\n");
    return NULL;
  }

  // Microdia tiene 2 interfaces
  if (usb_claim_interface(lvr_winusb, INTERFACE1) < 0) {
    fprintf(stderr, "Could not claim interface.\n");
    return NULL;
  }

  if (usb_claim_interface(lvr_winusb, INTERFACE2) < 0) {
    fprintf(stderr, "Could not claim interface.\n");
    return NULL;
  }

  return lvr_winusb;
}


static void read_product_string(usb_dev_handle *handle, struct usb_device *dev) {
  char prodIdStr[256];
  const struct product_hw *p;
  int strLen;

  memset(prodIdStr, 0, sizeof(prodIdStr));
  strLen = usb_get_string_simple(handle, dev->descriptor.iProduct, prodIdStr, sizeof(prodIdStr)-1);
  if (debug) {
    if (strLen < 0) {
      fprintf(stderr, "Couldn't read iProduct string");
    }
    else {
      fprintf(stderr, "iProduct: %s\n", prodIdStr);
    }
  }

  for (p = product_ids; p->id_string; ++p) {
    if (!strncmp(p->id_string, prodIdStr, sizeof(prodIdStr))) {
      break;
    }
  }
  tempOffset = p->offset;
}


usb_dev_handle *find_lvr_winusb() {
  struct usb_bus *bus;
  struct usb_device *dev;

  for (bus = usb_busses; bus; bus = bus->next) {
    for (dev = bus->devices; dev; dev = dev->next) {
      if (dev->descriptor.idVendor == VENDOR_ID && dev->descriptor.idProduct == PRODUCT_ID ) {
        usb_dev_handle *handle;
        if (debug) {
          fprintf(stderr, "lvr_winusb with Vendor Id: %x and Product Id: %x found.\n", VENDOR_ID, PRODUCT_ID);
        }

        if (!(handle = usb_open(dev))) {
          fprintf(stderr, "Could not open USB device\n");
          return NULL;
        }
        read_product_string(handle, dev);
        return handle;
      }
    }
  }
  return NULL;
}


void ini_control_transfer(usb_dev_handle *dev) {
  int r,i;
  char question[] = { 0x01, 0x01 };

  r = usb_control_msg(dev, 0x21, 0x09, 0x0201, 0x00, (char *) question, 2, timeout);
  if (r < 0) {
    fprintf(stderr, "USB control write"); bad("USB write failed");
  }

  if (debug) {
    for (i=0; i < reqIntLen; i++) {
      fprintf(stderr, "%02x ", question[i] & 0xFF);
    }
    fprintf(stderr, "\n");
  }
}

void control_transfer(usb_dev_handle *dev, const char *pquestion) {
  int r,i;
  char question[reqIntLen];

  memcpy(question, pquestion, sizeof question);

  r = usb_control_msg(dev, 0x21, 0x09, 0x0200, 0x01, (char *) question, reqIntLen, timeout);
  if (r < 0) {
    fprintf(stderr, "USB control write"); bad("USB write failed");
  }

  if(debug) {
    for (i=0; i < reqIntLen; i++) {
      fprintf(stderr, "%02x ", question[i] & 0xFF);
    }
    fprintf(stderr, "\n");
  }
}

void interrupt_transfer(usb_dev_handle *dev) {
  int r,i;
  char answer[reqIntLen];
  char question[reqIntLen];
  for (i=0; i < reqIntLen; i++) {
    question[i]=i;
  }
  r = usb_interrupt_write(dev, endpoint_Int_out, question, reqIntLen, timeout);
  if (r < 0) {
    fprintf(stderr, "USB interrupt write"); bad("USB write failed");
  }
  r = usb_interrupt_read(dev, endpoint_Int_in, answer, reqIntLen, timeout);
  if (r != reqIntLen) {
    fprintf(stderr, "USB interrupt read1"); bad("USB read failed");
  }

  if (debug) {
    for (i=0; i < reqIntLen; i++) {
      fprintf(stderr, "%i, %i, \n", question[i], answer[i]);
    }
  }

  usb_release_interface(dev, 0);
}

void interrupt_read(usb_dev_handle *dev) {
  int r,i;
  unsigned char answer[reqIntLen];
  bzero(answer, reqIntLen);

  r = usb_interrupt_read(dev, 0x82, (char*)answer, reqIntLen, timeout);
  if (r != reqIntLen) {
    fprintf(stderr, "r=%d. expected %d.\n", r, reqIntLen);
    fprintf(stderr, "USB interrupt read2"); bad("USB read failed");
  }

  if (debug) {
    for (i=0; i < reqIntLen; i++) {
      fprintf(stderr, "%02x ", answer[i] & 0xFF);
    }
    fprintf(stderr, "\n");
  }
}

float interrupt_read_temperature(usb_dev_handle *dev) {
  int r,i;
  unsigned char answer[reqIntLen];
  bzero(answer, reqIntLen);

  r = usb_interrupt_read(dev, 0x82, (char*)answer, reqIntLen, timeout);
  if (r != reqIntLen) {
    fprintf(stderr, "USB interrupt read3"); bad("USB read failed");
  }

  if (debug) {
    for (i=0; i < reqIntLen; i++) {
      fprintf(stderr, "%02x ", answer[i] & 0xFF);
    }
    fprintf(stderr, "\n");
  }

  /* Temperature in C is a 16-bit signed fixed-point number, big-endian */
  return (float)(signed char)answer[tempOffset] + answer[tempOffset+1] / 256.0f;
}

int main(int argc, char **argv) {
  usb_dev_handle *lvr_winusb = NULL;
  if ((lvr_winusb = setup_libusb_access()) == NULL) {
    exit(EXIT_FAILURE);
  }

  ini_control_transfer(lvr_winusb);

  control_transfer(lvr_winusb, uTemp);
  interrupt_read(lvr_winusb);

  control_transfer(lvr_winusb, uIni1);
  interrupt_read(lvr_winusb);

  control_transfer(lvr_winusb, uIni2);
  interrupt_read(lvr_winusb);
  interrupt_read(lvr_winusb);

  control_transfer(lvr_winusb, uTemp);
  float temp_c = interrupt_read_temperature(lvr_winusb);

  printf("%f\n", temp_c);
  fflush(stdout);

  usb_release_interface(lvr_winusb, INTERFACE1);
  usb_release_interface(lvr_winusb, INTERFACE2);

  usb_close(lvr_winusb);

  return 0;
}
