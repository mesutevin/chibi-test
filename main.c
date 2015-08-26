#include "hal.h"
#include "ch.h"
#include "test.h"
#include "math.h"

#define N 16

#pragma OPTIMIZE OFF


typedef struct
{
    char data[30];
}basicMsg;

memory_pool_t pool;
basicMsg data[N] __attribute__((aligned(sizeof(stkalign_t))));

msg_t kutu[64];
const char mask[4][30] = {{'1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1'},
                    {'2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2'},
                    {'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3'},
                    {'4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4'}};

static THD_WORKING_AREA(readerArea,128);/*boyut tamamen sallama*/
static THD_WORKING_AREA(worker1Area,128);/*boyut hala sallama*/
static THD_WORKING_AREA(squareWaveWA, 128);

static MAILBOX_DECL(mailman, &kutu,64);

static int isEqual(char *val1, char *val2,int count)
{
    int i;
    for(i = 0; i < count; i++)
    {
        if(val1[i] != val2[i])
        {
            return 0;
        }
    }

    return 1;
}

static void squareWave(void *arg)
{
    /* PB0 will be used for square wave
        use AlternateFunction.0 for digital io
    */

    palSetPadMode(GPIOB, 0, PAL_MODE_ALTERNATE(0));
	palSetPadMode(GPIOB, 0, PAL_MODE_OUTPUT_PUSHPULL);


    while(true)
    {
        //systime_t wakeTime = chVTGetSystemTimeX();
        //uint32_t generator = (uint32_t) wakeTime;
        //generator = (ST2MS(generator) / 1000) % 10;
        uint32_t period = 1500; // usec
        palSetPad(GPIOB, 0);
        chThdSleepMicroseconds(period/2);
        palClearPad(GPIOB, 0);
        chThdSleepMicroseconds(period/2);
    }
}

static void thFunc1(void *arg)
{
    while(true)
    {
        palSetPad(GPIOD, GPIOD_LED3);
        chThdSleepMilliseconds(1);
        palClearPad(GPIOD, GPIOD_LED3);
        chThdSleepMilliseconds(1);
    }

    basicMsg *message;
    msg_t buffer, retVal;
    while (!0)
    {
        retVal = chMBFetch(&mailman, (msg_t*)&buffer, 10);
        if(retVal == (msg_t)MSG_TIMEOUT)
        {
            chThdSleepMilliseconds(50);
            continue;
        }
        message = (basicMsg *)buffer;
        //sdWrite(&SD2, (msg_t *)message->data, 30);
        if(isEqual(message->data,mask[0],30))
        {
            palSetPad(GPIOD, GPIOD_LED3);
            chThdSleepMilliseconds(100);
            palClearPad(GPIOD, GPIOD_LED3);
            chThdSleepMilliseconds(100);
        }
        else if(isEqual(message->data,mask[1],30))
        {
            palSetPad(GPIOD, GPIOD_LED4);
            chThdSleepMilliseconds(100);
            palClearPad(GPIOD, GPIOD_LED4);
            chThdSleepMilliseconds(100);
        }
        else if(isEqual(message->data,mask[2],30))
        {
            palSetPad(GPIOD, GPIOD_LED5);
            chThdSleepMilliseconds(100);
            palClearPad(GPIOD, GPIOD_LED5);
            chThdSleepMilliseconds(100);
        }
        else if(isEqual(message->data,mask[3],30))
        {
            palSetPad(GPIOD, GPIOD_LED6);
            chThdSleepMilliseconds(100);
            palClearPad(GPIOD, GPIOD_LED6);
            chThdSleepMilliseconds(100);
        }
        else
        {
            palSetPad(GPIOD, GPIOD_LED3);
            palSetPad(GPIOD, GPIOD_LED4);
            palSetPad(GPIOD, GPIOD_LED5);
            palSetPad(GPIOD, GPIOD_LED6);
            chThdSleepMilliseconds(100);

            palClearPad(GPIOD, GPIOD_LED3);
            palClearPad(GPIOD, GPIOD_LED4);
            palClearPad(GPIOD, GPIOD_LED5);
            palClearPad(GPIOD, GPIOD_LED6);
            chThdSleepMilliseconds(100);
        }//*/
        chPoolFree(&pool, message);
    }
    return;
}

static void thFunc2(void *arg)
{
    basicMsg *message;
    message = (basicMsg *)chPoolAlloc(&pool);
    /*30 karakter*/
    int nTransfer,i;

	while(!0)
	{
        char buf;
        nTransfer = sdReadTimeout(&SD2,&buf,1,15);
        if(nTransfer == 0)//timeouta girdiyse
        {
            //sdWrite(&SD2, &i,1);
            if(i == 30)//30 karakter okumuşsa yolla sayacı sıfırla yeni bir struct oluştur
            {
                chMBPost(&mailman, (msg_t)message,1000);//mailboxa nesnenin adresini yaz
                message = (basicMsg *)chPoolAlloc(&pool);
            }
            //eğer 30 karakter okumadıysa sadece sayacı sıfırla
            i = 0;
        }
        else
        {
            sdWrite(&SD2, &i,1);
             message->data[i++] = buf;
        }

        /*char buffer[30];

        nTransfer = sdReadTimeout(&SD2,buffer, 30, 50);//7500 tick bekle 8Mhz kristalde yaklaşık 1 milisaniyeden biraz daha az
        //bu sayede timeouta girerek gelem mesajı okuyacağız
        //nTransfer = sdRead(&SD2,buffer,30);/*1 karakter oku buffer'a yolla
        sdWrite(&SD2, &nTransfer,1);
        if(nTransfer == 30)
        {
            message = (basicMsg *)chPoolAlloc(&pool);
            if(message != NULL)
            {
                for(i = 0; i < 30; i++)
                    message->data[i] = (char)buffer[i];//copy content of input
                chMBPost(&mailman, (msg_t)message,1000);//nesnenin adresini yaz mailboxa
                //msg_t s = chMBPost(&mailman, (msg_t)buffer, 5000);
                //sdWrite(&SD2, &buffer, 30);
            }
        }
        else
            ;*/
	}
    return;
}

int main(void)
{
	halInit();
	chSysInit();

    chPoolObjectInit(&pool, sizeof(basicMsg), NULL);
    int i;

	for(i=0; i < N; i++)
        chPoolFree(&pool, &data[i]);

	thread_t *tp;


	sdStart(&SD2, NULL);
	palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOD, 12, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOD, 13, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOD, 14, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOD, 15, PAL_MODE_OUTPUT_PUSHPULL);
	/*usart2 ve ledler kullanıma hazır*/

    chThdCreateStatic(squareWaveWA, sizeof(squareWaveWA), HIGHPRIO, squareWave, NULL);
	chThdCreateStatic(readerArea, sizeof(readerArea), NORMALPRIO, thFunc1, NULL);
	tp = chThdCreateStatic(worker1Area, sizeof(worker1Area), NORMALPRIO, thFunc2, NULL);
	chThdWait(tp);

	return 0;
}
