#include "tx.h"
#include "txc.h"
<<<<<<< HEAD
#include "txclib.c"

=======
>>>>>>> symbol pool

static const char tx_help[] =
    "TX Compiler "TX_VERSIONS" \n"
    "Usage: tx [options...] file\n"
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
                        tx_error("no input file\n");
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
                    tx_error("unrecognized command line option \"%s\"",str);
            }
        }else{
            if(t->filename){
                tx_error("error files");
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
