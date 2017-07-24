
/*modeA bit6~bit5*/   //平均采样数
typedef enum {
    SAMPLE_AVERAGE_1,
    SAMPLE_AVERAGE_2,
    SAMPLE_AVERAGE_4,
    SAMPLE_AVERAGE_8,
    SAMPLE_AVERAGE_MAX = SAMPLE_AVERAGE_8
}AVERAGE;
    #define OUTPUT_RATE_OFFSET      2
    #define SAMPLE_AVERAGE_MASK 0x60
 /*输出速率*/
typedef enum {
    /* modeA bit4~bit2 | Hz */
    OUTPUT_RATE_0_75,
    OUTPUT_RATE_1_5,
    OUTPUT_RATE_3,
    OUTPUT_RATE_7_5,
    OUTPUT_RATE_15,    //defult
    OUTPUT_RATE_30,
    OUTPUT_RATE_75,
    OUTPUT_RATE_MAX = OUTPUT_RATE_75
}RATE;
    #define OUTPUT_RATE_MASK 0x1C
    #define MESURA_NORMAL_OFFSET    0
 /*测量模式*/
typedef enum {
    /* modeA bit1~bit0 | mesura */
    MESURA_NORMAL,      //正常测量，测两次取平均
    MESURA_POSITIVE,     //正向电流偏置
    MESURA_NEGITIVE,     //负向电流偏置
    MESURA_MAX = MESURA_NEGITIVE
}MESURA;
    #define MESURA_MASK 0x03

#define GAIN_OFFSET             5
    
 /*modeB bit7~bit5，输出增益配置*/
typedef enum {
    /* bit | recommended | gain */
    GAIN_0_88_1370,   //0.88Gs范围，  1370count/Gs
    GAIN_1_3_1090,     //defult
    GAIN_1_9_820,
    GAIN_2_5_660,
    GAIN_4_0_440,
    GAIN_4_7_390,
    GAIN_5_6_330,
    GAIN_8_1_230,
    GAIN_MAX = GAIN_8_1_230
}GAIN;
    #define GAIN_MASK 0xE0    

#define MODE_OFFSET             0
/*MODE reg bit1~bit0*/
typedef enum {
    /* bit | mode */
    MODE_CONTINOUS,
    MODE_SINGLE,
    MODE_IDLE,
    MODE_MAX = MODE_IDLE
}MODE;
    #define MODE_MASK 0x03

#define HMC5883L_REG_CONFIG_A   0x00
#define HMC5883L_REG_CONFIG_B   0x01
#define HMC5883L_REG_MODE       0x02

#define HMC5883L_REG_XDATA_MSB  0x03
#define HMC5883L_REG_XDATA_LSB  0x04
#define HMC5883L_REG_YDATA_MSB  0x05
#define HMC5883L_REG_YDATA_LSB  0x06
#define HMC5883L_REG_ZDATA_MSB  0x07
#define HMC5883L_REG_ZDATA_LSB  0x08

#define HMC5883L_REG_STATUS     0x09
#define HMC5883L_VERSION_ID_A   0x0A
#define HMC5883L_VERSION_ID_B   0x0B
#define HMC5883L_VERSION_ID_C   0x0C

enum {
    AXIS_X,
    AXIS_Y,
    AXIS_Z,
    AXIS_MAX = AXIS_Z
};


typedef struct hmc5893_data {
    unsigned char sample_average;
    unsigned char output_rate;
    unsigned char mesura;
    unsigned char gain;
    unsigned char mode;
    short axis[AXIS_MAX + 1];

    unsigned char is_self_test;
    unsigned char is_validation;
}HMC5893_DATA;


void set_mode(MODE mode);
void set_gain(GAIN gain);
void set_mesure(MESURA mesure);
void set_sample_average(AVERAGE average);
void set_rate(RATE rate);
void hmc5893_init(void);
unsigned char get_RDY(void);
unsigned char get_LOCK(void);
void hmc5893_para_init(HMC5893_DATA *para);
void continous_mesure(HMC5893_DATA* data);
void magnetic_mesure(HMC5893_DATA* data,char times);
void magnetic_profile_mesure(HMC5893_DATA* data);
HMC5893_DATA* simple_mesure(HMC5893_DATA* data);
void* scout_input(void) ;

