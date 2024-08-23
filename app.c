#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<stdlib.h>
#include<stdint.h>
#include<poll.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/timerfd.h>


#define WR_VALUE  _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)



int main()
{
int32_t value,number;
int fd;
int ret;
char read_data_torbasi [40];
char hata_torbasi[40];
char torba [40];
char *dir = "/dev/alperen";
char kernel_val [20];


fd = open (dir, O_RDWR);
perror(hata_torbasi);
printf("fd = %d",fd);
 struct pollfd pfd;

    if(fd!=-1)
{
	printf("\nchardev.c opened with read write Access\n");
}

    pfd.fd = fd;
    pfd.events = POLLIN | POLLRDNORM;


while(1) {

   for(int i=0;i<15;i++){
	printf("****");
   }

   printf("\n\nread icin 1\nwrite icin 2\nclose icin 3\nioctl islemleri icin 4\nPoll işlemi icin 5\nCihazla poll kullanarak haberleşmek icin 6\nuygulamayı kapatmak icin 7"
	    "\nYapmak istediginiz operasyon icin ilgili sayiyi giriniz: " );

int x;
scanf("%d",&x);
   while(getchar()!= '\n');

switch(x)  {

  case 1:
	if(read(fd,read_data_torbasi,40) == -1)
		perror("Okuma gerçekleşmedi. Hata kodu: ");
	printf("\nSürücüden okunan veri: %s \n",read_data_torbasi);
	break;

  case 2:
	printf("\n Yazmak istediginiz datayi giriniz:");
	scanf("%s",torba);

	if(write(fd, torba, sizeof(torba) ) ==-1 )
		perror(hata_torbasi);
	else
		printf("\nData basarili bir sekilde yazildi.\n");
	break;

  case 3:
	if(close(fd) != 0)
		printf("\nClose islemi basarisiz. Hata kodu = ");
	perror(hata_torbasi);
	printf("\n");
	break;
	
  case 4:
  	printf("\nioctl_wr ile timer talimatı vermek icin 1\nioctl_rd ile timer bilgisini almak icin 2 "
  	"\nİstenilen islem icin gerekli tuslamayi giriniz.");
  	int y;
  	scanf("%d",&y);
  	while (getchar() != '\n');
  	
  	switch(y)
  	{
  	
  	case 1://cihaza yazma
  		printf("\nYeni timer degerini giriniz :");
  		scanf("%d",&number);
  		ioctl(fd,WR_VALUE, (int32_t*) &number);
  		printf("\nGirilen timer degeri = %d\n",number);
  		break;
  	
  	case 2: //cihazdan okuma
  		ioctl(fd, RD_VALUE , (int32_t*) &value);
  		printf("\nOkunan veri: %d\n", value);
  		break;
  	
  	}
  	 
  break; //case 4  
  	
  case 5:
  	printf("Poll tetiklenmesi için bekleniyor..\n");

        // poll fonksiyonunu çağır (sonsuz süre bekle)
        ret = poll(&pfd, 1, -1);
        printf("pollden dönen değer : %d\n",ret);
    if (ret < 0) 
    {
        perror("poll failed");
        close(fd);
        return EXIT_FAILURE;
    }else if (ret == 0)
    {
	printf("Poll timeout içerisine girdi.");
	break;    
    }    
    // poll fonksiyonu tetiklendiğinde
    else if (pfd.revents & POLLIN) 
    {
        printf("Poll tetiklendi ve process uykudan uyandı!\n");
    }   
    break;  
  
  case 6:
	printf("\nCihaza gönderilecek ve işlendikten sonra dönecek  veriyi giriniz: ");
	scanf("%s",torba);

	if(write(fd, torba, sizeof(torba) ) ==-1 )
		perror(hata_torbasi);
	else
		printf("\nData basarili bir sekilde yazildi.\n");
		printf("Poll içerisine giriliyor\n");
		
		int ret1 = poll(&pfd,1,-1);
		
		printf("pfd.revents = %d\n\n\n",pfd.revents);
		if (ret1<0) //pfd.revents == 0
		    {
			perror("poll failed");
			close(fd);
			return EXIT_FAILURE;
		    }
	    else if (ret1 == 0)
		    {
			printf("Poll timeout içerisine girdi.\n");
			break;         
	   	    } 
	   	    else if (ret1 == 1)
		 {
			printf("Bir olay gerçekleşti ama ne?\n");
			        
	   	     
	    		 if (pfd.revents == pfd.revents & (POLLIN | POLLRDNORM) )
	    		{ 
			printf("Okunulacak veri girişi olmuş!\npfd.revent=%d\nret1=%d\n",pfd.events,ret1);
			if(read(fd,read_data_torbasi,40) == -1);
			perror(" ");
			printf("\nSürücüden okunan veri: %s \n",read_data_torbasi);
			pfd.revents =0;
			}
		}   
	    break; 
		
		
  case 7:  	
	return 0;

}//switch sonu

}//while sonu

return 0;

}//main sonu




