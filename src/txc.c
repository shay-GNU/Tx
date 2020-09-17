#include "tx.h"
#include "txc.h"
#include "txclib.c"


static const char tx_help[] =
    "TX Compiler "TX_VERSIONS" \n"
    "Usage: tx [options...] [-o filename] ...\n"
    "-v :tx version\n"
    "-o :output filename\n"
    "-h :tx option help\n"
	;

int complie_parse_args(tx_Complie *t, int n, char **args){
    char *str;
    int i;

    if(n == 1) goto help;
    for(i = 1; i<n; i++){
        str = args[i];
        if(str[0] == '-'){
            switch(str[1]){
                case 'o':
                    if(str[2] != '\0') goto error;
                    if(i +1 >= n)
                        tx_error("option -o ȱ�ٲ���");
                    i++;
                    t->outName = args[i];
                    break;
                case 'v':
                    if(str[2] != '\0') goto error;
                    printf("tx version "TX_VERSIONS);
                    exit(0);
                case 'h':
                    if(str[2] != '\0') goto error;
                help:
                    printf(tx_help);
                    exit(0);
                default:
                error:
                    tx_error("not parse option \"%s\"\ninput -h(help)",str);
            }
        }else{
            if(t->filename){
                tx_error("ֻ�ܷ���һ���ļ�");
            }else{
                t->filename = args[i];
            }
        }
    }
}



int main(int argc, char **argv){
    tx_Complie *s;
    
    s = tx_newComplie();
    complie_parse_args(s, argc, argv);
	tx_complie(s);
    if(!s->outName)
        s->outName = "a.out";
    
    build_bin(s->outName);
    return 0;
}
