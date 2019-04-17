typedef struct {
    int       width;
    int       height;

    struct {
    	int id;
    	int precision;
    	int table[64];
    } QT[4];

    int channelNum;
    struct {
        int id;
        int sampleRate_X;
        int sampleRate_Y;
        int QT_idx;
        int HT_AC_idx;
        int HT_DC_idx;
    } channel_info[4];

    int Ri;

    int encodedDataLen;
    char *encodedData;
} JFIF;