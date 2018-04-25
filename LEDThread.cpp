/**
 * Copyright (c) 2017, Autonomous Networks Research Group. All rights reserved.
 * Developed by:
 * Autonomous Networks Research Group (ANRG)
 * University of Southern California
 * http://anrg.usc.edu/
 *
 * Contributors:
 * Jason A. Tran <jasontra@usc.edu>
 * Bhaskar Krishnamachari <bkrishna@usc.edu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * with the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * - Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimers.
 * - Redistributions in binary form must reproduce the above copyright notice, 
 *     this list of conditions and the following disclaimers in the 
 *     documentation and/or other materials provided with the distribution.
 * - Neither the names of Autonomous Networks Research Group, nor University of 
 *     Southern California, nor the names of its contributors may be used to 
 *     endorse or promote products derived from this Software without specific 
 *     prior written permission.
 * - A citation to the Autonomous Networks Research Group must be included in 
 *     any publications benefiting from the use of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH 
 * THE SOFTWARE.
 */

/**
 * @file       LEDThread.cpp
 * @brief      Implementation of thread that handles LED requests.
 *
 * @author     Jason Tran <jasontra@usc.edu>
 * @author     Bhaskar Krishnachari <bkrishna@usc.edu>
 */

#include "LEDThread.h"
#include "MQTTmbed.h"
#include "MQTTNetwork.h"

#include "MQTTClient.h"

Mail<MailMsg, LEDTHREAD_MAILBOX_SIZE> LEDMailbox;

extern void movement(char command, char speed, int delta_t);

static DigitalOut led2(LED2);

static const char *topic = "m3pi-mqtt-ee250/thread1";

int state;
int count=0;
float buf[4];

void LEDThread(void *args) 
{
    MQTT::Client<MQTTNetwork, Countdown> *client = (MQTT::Client<MQTTNetwork, Countdown> *)args;
    MailMsg *msg;
    MQTT::Message message;
    osEvent evt;
    char pub_buf[16];

    AnalogIn Ain(p20);
    float ADCdata;
    float distance;
    //state machine
    state=0;
    //buffer incrementer
    int a =0;
    //flag which data point was farthest
    int flag=0;
    int regress = 0;
    float max =0;
     //scan
    printf("begin\n");
    while(1)
    {
        
        if(state==0)
        {
            max=0;
            flag=0;
            a=0;
            regress = 0;
            while(count<=600)
            {
                 //ADCdata = V
                    
                ADCdata=Ain.read();
                //0.0032 V per cm
                //distance = cm
                distance = ADCdata * 2.38 / 0.0032; 
                buf[a]=distance;
                a++;
                printf("\n %d \n", a);

                printf("Distance: %f \n\r",distance);
                //increment sample rate
                count+=300;
                movement('d',22,count);

            }

            //find the largest distance and flag it
            for(int i=0;i<3;i++)
            {
                if(max<buf[i])
                {
                    max=buf[i];
                    flag=i;

                }
                printf("{%f YEEEt},\n",buf[i]);
                printf("current max is: %f\n",max);
           
            }

        
            regress = flag * count;

            printf("done");   
            printf("check");
            state=1;
            count=0;
            printf("this is the max value: %f",max);
        
        }                     
       if(state==1)
       {
            printf("did you stop here");
            movement('a',22,regress);
            state=3;
            printf("or hereee");
       }
       if(state==3)
       {
            while(1)
            {
                movement('w',50,500);
                wait(3);
                printf("or here");
            }
        }
        

    }



    while(1) {

        evt = LEDMailbox.get();

        if(evt.status == osEventMail) {
            msg = (MailMsg *)evt.value.p;
           
            /* the second byte in the message denotes the action type */
            switch (msg->content[1]) {
                case LED_THR_PUBLISH_MSG:
                    printf("LEDThread: received command to publish to topic"
                           "m3pi-mqtt-example/thread1\n");
                    pub_buf[0] = 'h';
                    pub_buf[1] = 'i';
                    message.qos = MQTT::QOS0;
                    message.retained = false;
                    message.dup = false;
                    message.payload = (void*)pub_buf;
                    message.payloadlen = 2; //MQTTclient.h takes care of adding null char?
                    /* Lock the global MQTT mutex before publishing */
                    mqttMtx.lock();
                    client->publish(topic, message);
                    mqttMtx.unlock();
                    break;
                case LED_ON_ONE_SEC:
                    printf("LEDThread: received message to turn LED2 on for"
                           "one second...\n");
                    led2 = 1;
                    wait(1);
                    led2 = 0;
                    break;
                case LED_BLINK_FAST:
                    printf("LEDThread: received message to blink LED2 fast for"
                           "one second...\n");
                    for(int i = 0; i < 10; i++)
                    {
                        led2 = !led2;
                        wait(0.1);
                    }
                    led2 = 0;
                    break;
                default:
                    printf("LEDThread: invalid message\n");
                    break;
            }            

            LEDMailbox.free(msg);
        }
    } /* while */

    /* this should never be reached */
}

Mail<MailMsg, LEDTHREAD_MAILBOX_SIZE> *getLEDThreadMailbox() 
{
    return &LEDMailbox;
}


