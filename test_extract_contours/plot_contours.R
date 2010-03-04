data = read.table("no_contours")

par(
   mfrow = c(2,1)
  )

x = ecdf(data$V1)
plot(x , xlab = "Number of contours in document", ylab = "cumulative % < x", main = '')

hist(data$V1, breaks = 100, probability = 1, xlab = "Number of contours in document", ylab = "Fraction", main = '')
