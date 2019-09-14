#output as png image
set terminal png

#save file to "domain.png"
set output "build/reports/benchmark/consecutive/plot.png"

#graph title
set title "5 000 consecutive requests"

#nicer aspect ratio for image size
set size 1,0.7

# y-axis grid
set grid y

#x-axis label
set xlabel "request"

#y-axis label
set ylabel "response time (ms)"

plot "build/reports/benchmark/consecutive/data.p" using 9 smooth sbezier with lines title "server"
