#include <comedilib.h>

#define SLOT0_INTERVAL  1200
#define FAST_INTERVAL   1200
#define MEDIUM_INTERVAL 10000
#define SLOW_INTERVAL   30000

#define SCXI_LINE_MOSI 0
#define SCXI_LINE_DA   1
#define SCXI_LINE_SS   2
#define SCXI_LINE_MISO 4

#define SCXI_DIO_NONE 0
#define SCXI_DIO_DO   1
#define SCXI_DIO_DI   2

#define SCXI_AIO_NONE 0
#define SCXI_AIO_AO   1
#define SCXI_AIO_AI   2

struct scxi_board_struct {
	unsigned int device_id;
	char name[12];
	int modclass;
	unsigned int clock_interval;
	int dio_type, aio_type, channels, status_reg, data_reg;
	int config_reg, eeprom_reg, gain_reg;
};

typedef struct scxi_board_struct scxi_board_t;

const scxi_board_t scxi_boards[] = {
	{ 0, "unknown\0", 2, SLOW_INTERVAL, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0xffffffff, "empty\0", 2, SLOW_INTERVAL, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0x06, "SCXI-1100\0", 1, FAST_INTERVAL, SCXI_DIO_NONE, SCXI_AIO_AI, 32,
		0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x1e, "SCXI-1102\0", 2, FAST_INTERVAL, SCXI_DIO_NONE, SCXI_AIO_AI, 32,
		0x02, 0x05, 0x01, 0x03, 0x04 },
	//{ 0x14, "SCXI-1124\0", 2, SLOW_INTERVAL, SCXI_DIO_NONE, SCXI_AIO_AO, 6,
	//	0x02, 0x08, 0x00, 0x03, 0x00 },
};

#define n_scxi_boards ((sizeof(scxi_boards)/sizeof(scxi_boards[0])))

struct scxi_module_struct {
	comedi_t *dev;
	unsigned int board;
	unsigned int dio_subdev, ser_subdev;
	unsigned int chassis, slot;
};

typedef struct scxi_module_struct scxi_mod_t;

#ifdef __cplusplus
extern "C" {
#endif

int comedi_scxi_serial_readwrite(comedi_t *it, unsigned char out_bits, unsigned char *in_bits);
int comedi_scxi_serial_config(comedi_t *it, unsigned int clock_interval);

#ifdef __cplusplus
}
#endif
