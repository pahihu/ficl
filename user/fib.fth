if (n < 2) return n;
else {
   \ a = fib(n-1);
   if (n < 3) a = n-1;
   else {
      fib3 = fib(n-3);        // n=3,4,5,6
      a    = fib(n-2) + fib3;
   }
   \ b = fib(n-2);
   if (n < 4) b = n-2;
   else {                     // n=4,5,6
      b = fib3 + fib(n-4);
   }
   return a+b;
}
