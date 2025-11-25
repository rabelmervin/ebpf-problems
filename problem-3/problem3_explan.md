### Problem statment 3:

    ```go
    package main

    import "fmt"

    func main() {
        cnp := make(chan func(), 10)
        for i := 0; i < 4; i++ {
            go func() {
                for f := range cnp {
                    f()
                }
            }()
        }
        cnp <- func() {
            fmt.Println("HERE1")
        }
        fmt.Println("Hello")
    }
    ```

# 1.Explaining how the highlighted constructs work?

 ```Make(chan func(), 10)``` 
 creates a channel where its type is just ```func()``` (it has no arguments and values) here 10 represents the size of buffer.

 ```for i := 0; i < 4; i++ { go func() { for f := range cnp { f() } }() }```
 the outer for loop creates 4 go routines where each go routine runs another function that for ```f := range cnp { f() }``` loops over each channel, getting ```func()``` until channel gets closed.

  ```for f := range cnp { f() }```

 range on a particular channel will receive values repeatedly until channel gets closed. when channel is closed the range exits/stops. Each received value is a f```unc()```, saved into f, and the code calls ```f()``` to execute the task.
 
# 2. Giving use-cases of what these constructs could be used for ?
 ```chan func()``` This is basically a channel where you send functions instead of data.You can use this when you want to sort of “hand over” some work to another goroutine without bothering with arguments.

```make(chan func(), 10)``` The buffer part is useful because it lets you queue multiple things before anyone picks them up.

loop starts 4 go routines- This is the typical pattern when you want multiple goroutines doing the same job.The 4 just means you can run up to 4 jobs at the same time.

```for f := range cnp { f() }``` Using range on the channel is a loop that waits for tasks as long as the channel is open.

# 3.Why the for loop runs 4 iterations ?

the 4 itertions responsible to create 4 go routines because more routines can give meore concurrency thus 4 tasks can run concurrently

# 4.What is the significance of make(chan func(), 10)?

In the code we have channel capacity 10 so, in the producer side ```(cnp <- task)``` will succeed immediately as long as the buffer is not full. Receiver side (workers) take items from the buffer when scheduled. suppose we set buffer to 0, a send blocks until some goroutine receives the value

# 5.Why "HERE1" is not getting printed ?

the channel was created with 10 capacity so, ```cnp <- func() { fmt.Println("HERE1") }``` will place the function into channel and return immediately. If none of the worker goroutines have had a chance to schedule and receive from the channel before main finishes. the print will not be executed
sometimes ```main()``` exits before the go routines run task.

to make the ```HERE``` print we can use ```WaitGroups```