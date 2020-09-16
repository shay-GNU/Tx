#include "tx.h"

static const char tx_help[] =
    "TX VM "TX_VERSIONS" \n"
    "Usage: tx [options...] ...\n"
    "-v :tx version\n"
    "-h :tx option help\n"
	;

int complie_parse_args(tx_State *t, int n, char **args){
    char *str;
    int i;

    if(n == 1) goto help;
    for(i = 1; i<n; i++){
        str = args[i];
        if(str[0] == '-'){
            switch(str[1]){
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
                    vm_error("not parse option \"%s\"\ninput -h(help)",str);
            }
        }else{
            if(t->filename){
                vm_error("ֻ��ִ��һ���ļ�");
            }else{
                t->filename = args[i];
            }
        }
    }
}


int main(int argc, char **argv){
    int main_ind;
    tx_State *s;

    s = tx_newState();
    vmname = argv[0];
    
    complie_parse_args(s, argc, argv);
    main_ind = load_bin(s, s->filename);
    
    
    load_main(s, main_ind);
    
    


    
    
    

    
    
    
    
    
    
    
    

    
    
    
    
    
    
    
    
    

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
}
