MIT License

Copyright (c) 2023 Mayur Pise (mayurpise@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


# Datagen

Introduction
Efficiently reading and writing files across various file systems is crucial for optimizing your infrastructure. To aid in this, I developed `datagen`, a versatile command-line tool designed to create complex directory and file structures. This tool simulates realistic usage scenarios, allowing you to measure read and write performance metrics across different types of file systems. 

Note that datagen is not intended to replace more extensive tools like fio but offers a quick and effective means to evaluate file system performance.

Overview of Datagen
`Datagen` helps in performing quick read and write performance tests on different file systems, whether local storage, NFS, SMB, or others. It generates a large number of files and directories, simulating real-world conditions to identify potential bottlenecks and areas for improvement. This makes it an invaluable tool for testing by creating different file and directory structures on various types of file systems.


## Command Usage

The basic usage of `datagen` is as follows:

```bash
./datagen
Usage: ./datagen <w|rw> <target_dir> <num_levels> <num_dirs_per_level> <num_files_per_dir> <file_size_kb> <target_level> <num_files_at_target_level>
Usage: ./datagen <r> <target_dir> <test_rounds>
Note: You can set and export NUM_THREADS, default is hardware concurrency
```

- `w`: Write only
- `rw`: Read and write both
- `r`: Read only
- `<test_rounds>`: Used with `r` to read all directory structure multiple rounds and take average
- `<target_dir>`: The directory where the test will be conducted
- `<num_levels>`: The depth of directory structure
- `<num_dirs_per_level>`: The number of directories per level
- `<num_files_per_dir>`: The number of files per directory
- `<file_size_kb>`: The size of each file in kilobytes
- `<target_level>`: The level at which specific operations are targeted
- `<num_files_at_target_level>`: The number of files at the target level

Additionally, you can set the number of threads by exporting NUM_THREADS, which defaults to the hardware concurrency if not set.

## Build
```bash
g++ -std=c++17 datagen.cpp -o datagen -lstdc++fs -pthread
```

## Example Usage

Let’s consider an example command:

```bash
./datagen rw /tmp/testdir 5 5 5 5 5 5
```

This command generates a structure with 5 levels, each containing 5 directories, with 5 files per directory, each file being 5 KB in size. The read and write tests will also be conducted at target level 5.

## Performance Metrics

After running the command, `datagen` outputs detailed performance metrics. Here’s a sample output:

```plaintext
Round 1 - Files: 19525, Dirs: 3905, Duration: 38 ms, Read IOPS: 616579, Response Time: 0.00162185 ms
Round 2 - Files: 19525, Dirs: 3905, Duration: 41 ms, Read IOPS: 571463, Response Time: 0.00174989 ms
Round 3 - Files: 19525, Dirs: 3905, Duration: 38 ms, Read IOPS: 616579, Response Time: 0.00162185 ms
Round 4 - Files: 19525, Dirs: 3905, Duration: 36 ms, Read IOPS: 650833, Response Time: 0.00153649 ms
Round 5 - Files: 19525, Dirs: 3905, Duration: 33 ms, Read IOPS: 710000, Response Time: 0.00140845 ms
Average - Files: 19525, Dirs: 3905, Duration: 37 ms, Read IOPS: 633090, Response Time: 0.00157917 ms

Target Directory:             /tmp/testdir
Total Dirs:                   3905
Total Files:                  19525
Total Files & Dirs:           23430
Write Duration ms:            2851
Write IOPS:                   8215
Write Response Time ms:       0.121728
Read Duration ms:             37
Read IOPS:                    633090
Read Response Time ms:        0.00157917
```

## Understanding the Metrics

- **Files and Dirs**: The total number of files and directories created.
- **Duration**: The time taken for the read or write operation.
- **IOPS (Input/Output Operations Per Second)**: A measure of how many I/O operations a storage system can process per second.
- **Response Time**: The average time taken to complete an I/O operation.

From the sample output:
- **Write IOPS**: 8215 with a response time of 0.121728 ms.
- **Read IOPS**: 633090 with a response time of 0.00157917 ms.

These metrics give a quick view of the file system’s performance, helping you identify any potential bottlenecks or areas for improvement.

## Creating Custom Directory Structures

With `datagen`, you can be creative or meet specific requirements by generating any kind of file and directory structure. This flexibility makes it an invaluable tool for benchmarking and testing file systems under various scenarios.

`datagen` allows for the creation of various complex directory structures tailored to specific requirements. Here are some examples of different types of directory structures you can create:

1. **Simple Read-Only Structure**:
   ```bash
   ./datagen w /target/dir1 1 2 3 10 1 3
   ```

2. **Moderate Read-Write Structure**:
   ```bash
   ./datagen rw /target/dir2 2 3 5 50 2 4
   ```

3. **Nested Structure with Small Files**:
   ```bash
   ./datagen w /target/dir3 3 2 2 5 3 6
   ```

4. **Wide Structure with Large Files**:
   ```bash
   ./datagen rw /target/dir4 1 5 10 100 1 5
   ```

5. **Deep Structure with Sparse Files**:
   ```bash
   ./datagen w /target/dir5 3 1 1 20 3 2
   ```

6. **Balanced Structure with Uniform Files**:
   ```bash
   ./datagen rw /target/dir6 2 3 3 30 2 3
   ```

7. **Complex Structure with Mixed Sizes**:
   ```bash
   ./datagen w /target/dir7 3 3 5 10 3 10
   ```

8. **Flat Structure with Many Files**:
   ```bash
   ./datagen rw /target/dir8 1 10 20 50 1 15
   ```

9. **Deep Nested with Large Sparse Files**:
   ```bash
   ./datagen w /target/dir9 3 2 2 100 3 1
   ```

10. **Two-Level Wide Structure**:
    ```bash
    ./datagen rw /target/dir10 2 4 4 25 2 4
    ```

11. **Three-Level Balanced Structure**:
    ```bash
    ./datagen w /target/dir11 3 3 3 15 3 5
    ```

12. **Shallow Wide Structure**:
    ```bash
    ./datagen rw /target/dir12 1 6 6 20 1 10
    ```

13. **Moderately Deep Structure**:
    ```bash
    ./datagen w /target/dir13 2 2 2 40 2 5
    ```

14. **Sparse Wide Structure**:
    ```bash
    ./datagen rw /target/dir14 1 4 4 30 1 2
    ```

15. **Complex Nested Structure**:
    ```bash
    ./datagen w /target/dir15 3 4 4 50 3 8
    ```

16. **Minimalist Nested Structure**:
    ```bash
    ./datagen rw /target/dir16 3 1 1 10 3 1
    ```

17. **Uniformly Populated Structure**:
    ```bash
    ./datagen w /target/dir17 2 2 2 20 2 2
    ```

18. **Deep Wide Structure**:
    ```bash
    ./datagen rw /target/dir18 3 4 4 25 3 4
    ```

19. **Balanced and Large Files**:
    ```bash
    ./datagen w /target/dir19 2 3 3 60 2 3
    ```

20. **Highly Nested Structure**:
    ```bash
    ./datagen rw /target/dir20 3 2 3 15 3 3
    ```

### Example Structures

1. **5 Files in a Single Directory on Level 2**
   ```bash
   ./datagen rw /target/dir1 2 1 0 10 2 5
   ```

   ```plaintext
    /target/dir1/
    └── dir_1_0
        └── dir_2_0
            ├── file_0
            ├── file_1
            ├── file_2
            ├── file_3
            └── file_4
   ```

2. **5 Files in a Single Directory on Level 6**
   ```bash
   ./datagen rw /target/dir2 6 1 0 10 6 5
   ```

   ```plaintext
    /target/dir2
    └── dir_1_0
        └── dir_2_0
            └── dir_3_0
                └── dir_4_0
                    └── dir_5_0
                        └── dir_6_0
                            ├── file_0
                            ├── file_1
                            ├── file_2
                            ├── file_3
                            └── file_4
   ```

3. **5k Files in a Single Directory on Level 11**
   ```bash
   ./datagen rw /target/dir3 11 1 0 10 11 5
   ```

   ```plaintext
    /target/dir3
    └── dir_1_0
        └── dir_2_0
            └── dir_3_0
                └── dir_4_0
                    └── dir_5_0
                        └── dir_6_0
                            └── dir_7_0
                                └── dir_8_0
                                    └── dir_9_0
                                        └── dir_10_0
                                            └── dir_11_0
                                                ├── file_0
                                                ├── file_1
                                                ├── file_2
                                                ├── file_3
                                                └── file_4
   ```

4. **5k Files, Each in Its Own Directory on Level 2**
   ```bash
   ./datagen rw /target/dir4 1 5 1 10 2 1
   ```

   ```plaintext
    /target/dir4
    ├── dir_1_0
    │   └── file_0
    ├── dir_1_1
    │   └── file_0
    ├── dir_1_2
    │   └── file_0
    ├── dir_1_3
    │   └── file_0
    └── dir_1_4
        └── file_0
   ```

5. **5 Files, Each in Its Own Directory on Level 6**
   ```bash
   ./datagen rw /target/dir5 6 1 1 10 6 1
   ```

   ```plaintext
    /target/dir5/
    └── dir_1_0
        ├── dir_2_0
        │   ├── dir_3_0
        │   │   ├── dir_4_0
        │   │   │   ├── dir_5_0
        │   │   │   │   ├── dir_6_0
        │   │   │   │   │   └── file_0
        │   │   │   │   └── file_0
        │   │   │   └── file_0
        │   │   └── file_0
        │   └── file_0
        └── file_0
   ```

6. **Files each in Its Own Directory and 5 files on Level 11**
   ```bash
   ./datagen rw /target/dir6 11 1 0 10 11 5
   ```

   ```plaintext
    /target/dir6/
    └── dir_1_0
        ├── dir_2_0
        │   ├── dir_3_0
        │   │   ├── dir_4_0
        │   │   │   ├── dir_5_0
        │   │   │   │   ├── dir_6_0
        │   │   │   │   │   ├── dir_7_0
        │   │   │   │   │   │   ├── dir_8_0
        │   │   │   │   │   │   │   ├── dir_9_0
        │   │   │   │   │   │   │   │   ├── dir_10_0
        │   │   │   │   │   │   │   │   │   ├── dir_11_0
        │   │   │   │   │   │   │   │   │   │   ├── file_0
        │   │   │   │   │   │   │   │   │   │   ├── file_1
        │   │   │   │   │   │   │   │   │   │   ├── file_2
        │   │   │   │   │   │   │   │   │   │   ├── file_3
        │   │   │   │   │   │   │   │   │   │   └── file_4
        │   │   │   │   │   │   │   │   │   └── file_0
        │   │   │   │   │   │   │   │   └── file_0
        │   │   │   │   │   │   │   └── file_0
        │   │   │   │   │   │   └── file_0
        │   │   │   │   │   └── file_0
        │   │   │   │   └── file_0
        │   │   │   └── file_0
        │   │   └── file_0
        │   └── file_0
        └── file_0
   ```

7. **3 Files, Each in Its Own Path from the Root, Path 2 Levels Deep**
   ```bash
   ./datagen rw /target/dir7 2 5000 1 10 2 1
   ```

   ```plaintext
    /target/dir7/
    ├── dir_1_0
    │   ├── dir_2_0
    │   │   └── file_0
    │   ├── dir_2_1
    │   │   └── file_0
    │   ├── dir_2_2
    │   │   └── file_0
    │   └── file_0
    ├── dir_1_1
    │   ├── dir_2_0
    │   │   └── file_0
    │   ├── dir_2_1
    │   │   └── file_0
    │   ├── dir_2_2
    │   │   └── file_0
    │   └── file_0
    └── dir_1_2
        ├── dir_2_0
        │   └── file_0
        ├── dir_2_1
        │   └── file_0
        ├── dir_2_2
        │   └── file_0
        └── file_0
   ```

8. **Files in different levels in directory structure**
   ```bash
   ./datagen rw /target/dir8 6 5000 1 10 6 1
   ```

   ```plaintext
    /target/dir8/
    └── dir_1_0
        └── dir_2_0
            ├── dir_3_0
            │   └── dir_4_0
            │       ├── dir_5_0
            │       │   └── dir_6_0
            │       │       └── dir_7_0
            │       │           └── dir_8_0
            │       │               ├── dir_9_0
            │       │               │   └── dir_10_0
            │       │               │       └── dir_11_0
            │       │               │           ├── file_0
            │       │               │           └── file_1
            │       │               ├── file_0
            │       │               ├── file_1
            │       │               ├── file_2
            │       │               ├── file_3
            │       │               └── file_4
            │       ├── file_0
            │       ├── file_1
            │       ├── file_2
            │       ├── file_3
            │       └── file_4
            ├── file_0
            ├── file_1
            ├── file_2
            ├── file_3
            └── file_4
    ```

## Practical Applications

`datagen` is ideal for:
- **Performance Benchmarking**: Quickly assessing the performance of various file systems under different load conditions.
- **Comparative Analysis**: Comparing the performance of local storage versus network-mounted file systems (NFS, SMB).
- **Capacity Planning**: Understanding how your file system performs under heavy load to plan for future capacity needs.
- **System Optimization**: Identifying and mitigating performance issues by analyzing read and write response times.

## Conclusion

`Datagen` is a powerful and versatile tool for anyone looking to measure and understand the performance characteristics of their file systems. By simulating real-world usage scenarios through the creation of complex directory and file structures, it provides valuable insights that can drive improvements in system design and configuration. Whether you are assessing the performance of local storage, NFS, SMB, or other file systems, datagen offers a quick and effective way to conduct read and write performance tests, identify potential bottlenecks, and optimize your infrastructure. 

Try out datagen on your file systems, share your findings, and use the data to enhance your storage solutions and overall system performance. Happy benchmarking!.

If you have any questions or need further assistance with datagen, feel free to reach out in the comments or contact me directly at mayurpise@gmail.com. Your feedback is highly appreciated!


