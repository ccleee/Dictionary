#include <stdio.h>
#include <fcntl.h>
#include <string.h>
int main()
{
    FILE *fp1,*fp2;
    char temp[64],*p;
    if((fp1 = fopen("E-R.txt","r")) == NULL){
        perror("fp1:");
        return -1;
    }
    if((fp2 = fopen("temp.txt","w+")) == NULL){
        perror("fp2");
        return -1;
    }
    while(fgets(temp,64,fp1) != NULL){
        if(temp[1] != '1'&& temp[0] != '<' && temp[0] != ' '){

            int len = strlen(temp);
            temp[len-1] = '\0';
            temp[len-2] = '\0';
            
            if(temp[1] == '4'){
                p = temp;
                p = p + 3;
                fprintf(fp2,"%s\n",p);
            }
            else{
                fprintf(fp2,"%-25s",temp);
            }
        }
    }
    return 0;
}

