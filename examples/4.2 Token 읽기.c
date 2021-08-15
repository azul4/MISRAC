void temp()
{
state1:
    ch = nextChar();
    k = 0;
    if(charClassT[ch] == letter) {
        goto state2;
    } else {
        error();
    }

state2:
    a[k++] = ch;
    ch = nextChar();
    if(charClassT[ch] == letter || charClassT[ch] == digit) {
        goto state2;
    } else {
        goto state3;
    }

state3:
    backChar();
}

void backChar()
{
    lineIndex--;
}

void ReadToken()
{
    while(ch == ' ') {
        ch = nextChar();
    }

state1:
    if(charClassT[ch] == letter) goto state2;
    if(charClassT[ch] == digit ) goto state3;
    if(charClassT[ch] == delimiter) goto state4;
    error();

state2:
    ch = nextChar();
    if(charClassT[ch] == letter || charClassT[ch] == digit) {
        goto state2;
    } else {
        goto state5;
    }

state3:
    ch = nextChar();
    if (charClassT[ch] == digit) {
        goto state3;
    } else {
        goto state5;
    }
    
state4:
    ch = nextChar();
    goto state5;

state5:
}