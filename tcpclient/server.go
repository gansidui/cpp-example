package main

import (
	"fmt"
	"log"
	"net"
)

func handleConn(conn *net.TCPConn) {
	addr := conn.RemoteAddr()
	defer func() {
		recover()
		conn.Close();
		fmt.Println("close:", addr)
	}()
	fmt.Println("accept:", addr)

	// read
	buf := make([]byte, 1024)
	n, err := conn.Read(buf)
	if err != nil {
		return
	}
	fmt.Println(string(buf[0:n]))

	// write
	conn.Write([]byte("hellohellohello"))
}

func main() {
	tcpAddr, err := net.ResolveTCPAddr("tcp4", "127.0.0.1:8989")
	checkError(err)
	listener, err := net.ListenTCP("tcp", tcpAddr)
	checkError(err)
	fmt.Println("listening:", listener.Addr())

	for {
		conn, err := listener.AcceptTCP()
		if err != nil {
			continue
		}
		go handleConn(conn)
	}
}

func checkError(err error) {
	if err != nil {
		log.Fatal(err)
	}
}
