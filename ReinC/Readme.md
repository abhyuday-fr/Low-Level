# ReinC
A Reinforcement Learning Model made in C

## Some Revision of the concepts

### Forward Pass
In the [drawio](neural_network_flowchart.drawio) of forward pass, it is shown how an input vector x gets transformed step-by-step into output class probabilities.

**Layer 1 (hidden layer 1):**

1. Input x : the input vector goes in.
2. Matmul (W0 × x) : multiply the input by weight matrix W0.
3. Add (b0) : add the bias vector b0 to that result.
4. Activation / Z1 : pass the result through a non-linear activation function (like ReLU or sigmoid) to get Z1, the output of the first hidden layer.

**Layer 2 (hidden layer 2):**

5. Matmul (W1 × Z1) : multiply Z1 by the second weight matrix W1.
6. Add (b1) : add bias b1.
7. Activation / Z2 : apply activation again to get Z2, the second hidden layer's output.

**Output layer:**

8. Matmul (W2 × Z2) : multiply Z2 by the final weight matrix W2.
9. Add (b2) : add the final bias b2. This produces the raw output scores, often called logits.
10. Softmax : convert those logits into a probability distribution (values between 0 and 1 that sum to 1).
11. Output probabilities : the final result: a probability for each class.
