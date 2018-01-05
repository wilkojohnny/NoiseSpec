#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr;

#include <noisespecmaths.h>

int round2(int number)
{
    return pow(2,ceil(log(number)/log(2)));
}

void four1(float * data, unsigned int nn, int isign)
{
    // Replaced all [x] with [x-1]... so now arrays start at 0

    unsigned long n,mmax,m,j,istep,i;
    double wtemp,wr,wpr,wpi,wi,theta; //Double precision for the trigonometfloat
    float tempr,tempi; //ric recurrences.
    n=nn << 1;
    j=1;// replaced 1<->0
    for (i=1;i<n;i+=2) { // replaced i=1<->0
        if (j > i) {
            SWAP(data[j-1],data[i-1]); //Exchange the two complex numbers.
            SWAP(data[j],data[i]);
        }
        m=nn;
        while (m >= 2 && j > m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }

    mmax=2;
    while (n > mmax) { //Outer loop executed log2 nn times.
        istep=mmax << 1;
        theta=isign*(6.28318530717959/mmax); //Initialize the trigonometric recurrence.
        wtemp=sin(0.5*theta);
        wpr = -2.0*wtemp*wtemp;
        wpi=sin(theta);
        wr=1.0;
        wi=0.0;
        for (m=1;m<mmax;m+=2) {// Here are the two nested inner loops. // replaced m=1<->0
            for (i=m;i<=n;i+=istep) {
                j=i+mmax;// This is the Danielson-Lanczos fortempr=wr*data[j]-wi*data[j+1];
                tempi=wr*data[j]+wi*data[j-1];
                data[j-1]=data[i-1]-tempr;
                data[j]=data[i]-tempi;
                data[i-1] += tempr;
                data[i] += tempi;
            }
            wr=(wtemp=wr)*wpr-wi*wpi+wr;// Trigonometric recurrence.
            wi=wi*wpr+wtemp*wpi+wi;
        }
        mmax=istep;
    }


}

void Sort(QList<int> &Data) {
    // Bubble sort routine

    int tempr; // var for SWAP
    int noSwaps; // no swaps counter

    do {
        noSwaps = 0;

        // for each pair...
        for (int i=0; i<(Data.size()-1); ++i){
            // ... swap if previous is greater than current
            if (Data[i]>Data[i+1]) {
                noSwaps++;
                SWAP(Data[i], Data[i+1]);
            }
        }
    } while (noSwaps!=0);
}
