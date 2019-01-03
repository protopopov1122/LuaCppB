fib = fibonacci(15)
sum = 0
for k, v in pairs(fib) do
  sum = sum + v
end
print('Fibonacci  sum is ' .. sum)
print('5! is ' .. factorial(5))