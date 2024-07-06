// gusse... But I found my label naming problem and I passed after I solved the repeated label name
//(I used to name if or while begin with 1 thus may cause repeat when 2 more functions has if sentence or while sentence). 

int fibonacci(int n) {
    if (n <= 1) {
        println_int(1);
        return n;
    } else {
        println_int(n);
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

int main() {
    int n;
    n = 5;
    if(n>10){
        int result = 0, i = 1, j = 1;
        int tmp = 0;
        while(j <= n){
            result = i + j ;
            tmp = j ;
            j = j + i;
            i = tmp;
            println_int(result);
        }
    }else{
        fibonacci(n);
    }
    
    return 0;
}











