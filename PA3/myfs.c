#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"
#define YEL  "\x1B[33m"
#define BLU  "\x1B[34m"
#define CYN  "\x1B[36m"
#define WHT  "\x1B[37m"

FILE *fp;
FILE *fp2;

unsigned char FAT[4096][4];
unsigned char File_List[128][256];
unsigned char Data[4096][512];
//In total 2146304 char(byte)
char file_name[248];

unsigned int i,j,k;
unsigned int file_size;
unsigned int allocated_block_size,write_size;
unsigned int first_block, current_block, next_block;



void Format(){
fseek(fp, 0, SEEK_SET);
for (i = 0; i<4; i++) {
    fputc(0xFF, fp);
}
for (i = 0; i<2146300; i++) {
    fputc(0x00, fp);
}
printf(GRN"Disk was formatted succesfully \x0A");
}



void Write(char *srcPath, char *destFileName){
fp2 = fopen(srcPath,"r+");
fseek(fp2, 0, SEEK_END);
file_size= ftell(fp2);
allocated_block_size = (file_size/512)+1;

fseek(fp2, 0, SEEK_SET);

for (j=1;j<4096;j++){
    if (FAT[j][0]==0 && FAT[j][1]==0 && FAT[j][2]==0 && FAT[j][3]==0){
        first_block=j; break;
    }
}

for (i=1;i<allocated_block_size;i++){
    for (j=1;j<4096;j++){
        if (FAT[j][0]==0 && FAT[j][1]==0 && FAT[j][2]==0 && FAT[j][3]==0){
            current_block=j; break;
        }
    }
    for (j=current_block+1;j<4096;j++){
        if (FAT[j][0]==0 && FAT[j][1]==0 && FAT[j][2]==0 && FAT[j][3]==0){
            next_block=j; break;
        }
    }

    FAT[current_block][0]=(next_block & 0xFF);
    FAT[current_block][1]=((next_block >> 8) & 0xFF);
    FAT[current_block][2]=((next_block >> 16) & 0xFF);
    FAT[current_block][3]=((next_block >> 24) & 0xFF);
    fread(Data[current_block],512, 1, fp2);
}

for (j=1;j<4096;j++){
    if (FAT[j][0]==0 && FAT[j][1]==0 && FAT[j][2]==0 && FAT[j][3]==0){
        FAT[j][0]=0xFF;
        FAT[j][1]=0xFF;
        FAT[j][2]=0xFF;
        FAT[j][3]=0xFF;
        fread(Data[j], 512, 1, fp2);  break;
    }
}

for(i=1;i<128;i++){
    if(File_List[i][0]==0 && File_List[i][1]==0 && File_List[i][2]==0 && File_List[i][3]==0){
        File_List[i][0]=(file_size & 0xFF);
        File_List[i][1]=((file_size >> 8) & 0xFF);
        File_List[i][2]=((file_size >> 16) & 0xFF);
        File_List[i][3]=((file_size >> 24) & 0xFF);
        File_List[i][4]=(first_block & 0xFF);
        File_List[i][5]=((first_block >> 8) & 0xFF);
        File_List[i][6]=((first_block >> 16) & 0xFF);
        File_List[i][7]=((first_block >> 24) & 0xFF);
        for(j=0;j<strlen(destFileName);j++){
            File_List[i][j+8]=destFileName[j];
        }
        break;
    }
}

fseek(fp, 0, SEEK_SET);
fwrite(FAT,sizeof(FAT),1,fp);
fwrite(File_List,sizeof(File_List),1,fp);
fwrite(Data,sizeof(Data),1,fp);

printf(CYN"File was copied to disk succesfully \x0A");
}



void Read(char *srcFileName, char *destPath){
fp2 = fopen(destPath,"w+");

for(i=1;i<256;i++){
    k=0;
    for(j=0;j<strlen(srcFileName);j++){
        if (File_List[i][j+8]==srcFileName[j]){
        k++;}
    }
    if (k==strlen(srcFileName)){
    first_block = (File_List[i][7]<<24) | (File_List[i][6]<<16) | (File_List[i][5]<<8) | (File_List[i][4]);
    file_size = (File_List[i][3]<<24) | (File_List[i][2]<<16) | (File_List[i][1]<<8) | (File_List[i][0]);
    break;
    }
}

current_block=first_block;
int fileSize=file_size;
for(i=0;fileSize>0;i++){
    if(fileSize<512){write_size=fileSize;} else {write_size=512;}
    fileSize-=512;
    fwrite(Data[current_block],write_size,1,fp2);
    next_block = (FAT[current_block][3]<<24) | (FAT[current_block][2]<<16) | (FAT[current_block][1]<<8) | (FAT[current_block][0]);
    current_block=next_block;
}

fclose(fp2);

printf(BLU"File was copied from disk succesfully \x0A");
}



void Delete(char *filename){
for(i=1;i<256;i++){
    k=0;
    for(j=0;j<strlen(filename);j++){
        if (File_List[i][j+8]==filename[j]){
        k++;}
    }
    if (k==strlen(filename)){
    first_block = (File_List[i][7]<<24) | (File_List[i][6]<<16) | (File_List[i][5]<<8) | (File_List[i][4]);
    file_size = (File_List[i][3]<<24) | (File_List[i][2]<<16) | (File_List[i][1]<<8) | (File_List[i][0]);
    for(int m=0;m<256;m++){
    File_List[i][m]=0x00;}  break;
    }
}
current_block=first_block;
int fileSize=file_size;
for(i=0;fileSize>0;i++){
    if(fileSize<512){write_size=fileSize;} else {write_size=512;}
    fileSize-=512;
    next_block = (FAT[current_block][3]<<24) | (FAT[current_block][2]<<16) | (FAT[current_block][1]<<8) | (FAT[current_block][0]);
    for(k=0;k<4;k++){
        FAT[current_block][k]=0x00;}
    for(k=0;k<512;k++){
        Data[current_block][k]=0x00;}
    current_block=next_block;
}

fseek(fp, 0, SEEK_SET);
fwrite(FAT,sizeof(FAT),1,fp);
fwrite(File_List,sizeof(File_List),1,fp);
fwrite(Data,sizeof(Data),1,fp);

printf(YEL"File was deleted from disk succesfully \x0A");
}



void List(){
printf(RED"%-50s%-12s\x0A","File Name","File Size"); // I showed first 50 character in the name to fit the window, it can be changed from this line.
k=0;
for(i=0;i<128;i++){
    file_size = ((File_List[i][3]<<24) | (File_List[i][2]<<16) | (File_List[i][1]<<8) | (File_List[i][0]));
    if (file_size != 0){
        for(j=0;j<248;j++){
        file_name[j]=File_List[i][j+8];
        }
    printf(GRN"%-50s",file_name);
    printf(GRN"%-12u \x0A",file_size); k++;
    }
}
printf(YEL"There are %d files in the disk\x0A",k);
}



int main(int argc, char *argv[])
{
    fp = fopen(argv[1],"r+");
    fseek(fp, 0, SEEK_SET);

    fread(FAT, sizeof(FAT), 1, fp);
    fread(File_List, sizeof(File_List), 1, fp);
    fread(Data, sizeof(Data), 1, fp);

    if (!strcmp(argv[2],"-format") && argc==3) {
    Format();
    }
    else if (!strcmp(argv[2],"-write")&& argc==5){
    Write(argv[3], argv[4]);
    }
    else if (!strcmp(argv[2],"-read")&& argc==5){
    Read(argv[3], argv[4]);
    }
    else if (!strcmp(argv[2],"-delete")&& argc==4){
    Delete(argv[3]);
    }
    else if (!strcmp(argv[2],"-list")&& argc==3){
    List();
    }
    else {
    printf(RED"Wrong Input\x0A");
    }

    fclose(fp);
    return 0;
}
