// go battery
package main

import (
	"fmt"
	"math/rand"
	"time"
)

var s1 = rand.NewSource(time.Now().UnixNano())
var r1 = rand.New(s1)


type Battery struct {
   	name string
   	id int
   	capacity float32
   	cap_used float32
   	//soc float32
   	max_volt float32
	min_volt float32
	temp float32
}

func (b *Battery) setup(name string, id int) int {
	b.name = name
	b.id = id
	b.capacity = 4700.0 + (r1.Float32()*200.0)
	b.cap_used = 0.0
	b.max_volt = 4.2
	b.min_volt = 2.5
	b.temp = 22.4 + (r1.Float32()*2.0)
	//b.soc = 100.0
	return id
}

func (b *Battery) discharge( mA float32, ms float32)  {
	b.cap_used += mA * ms
	if b.cap_used > b.capacity {
		b.cap_used = b.capacity
	}
}

func (b *Battery) charge( mA float32, ms float32)  {
	b.cap_used -= (mA * ms) * 0.9
	if b.cap_used < 0.0 {
		b.cap_used = 0.0
	}
}

func (b *Battery) soc() float32 {
	return (b.capacity-b.cap_used)/ (.0100 * b.capacity)
}

func (b *Battery) volts() float32 {
	s:= (100.0 -b.soc())/100.0
	v:= b.max_volt - (s * s) * (b.max_volt - b.min_volt) 
	return v
}

func tvolts (array []Battery) float32 {
	var tvolts float32
	tvolts = 0.0

	for i := 0 ; i <len(array) ; i++ {
		b := array[i]
		tvolts += b.volts()
		//Bats[1].setup (" Battery 2", 2)
	   }	
	return tvolts;
}


func main() {
	var Bats = make([] Battery, 64)
    var Containers = make(map[string]map[string][]Battery)
   /* Battery 1 specification */
   Containers["main"] = make (map[string][]Battery)
   Containers ["main"]["bats1"] = make([] Battery, 64)
   Containers ["main"]["bats2"] = Bats
   for a,b := range Containers{
	   fmt.Printf(" Container [%s] :", a)
	   for c,_ := range b {
		fmt.Printf(" Sub [%s] :", c)

	   }
	   fmt.Printf(" \n")

   }
   fmt.Printf(" \n")

   for i := 0 ; i <len(Bats) ; i++ {
	//b := Bats[i]
	Bats[i].setup(" battery", i)
	fmt.Printf( "Bat id : %d cap %f soc %f \n", Bats[i].id,  Bats[i].capacity, Bats[i].soc())
	//Bats[1].setup (" Battery 2", 2)
   }
   /* Battery 1 specification */
//    for i := 0 ; i <len(Bats) ; i++ {
// 	b := Bats[i]
// 	//b.setup(" battery", i)
// 	fmt.Printf( "Bat id : %d cap %f soc %f \n", b.id,  b.capacity, b.soc())
// 	//Bats[1].setup (" Battery 2", 2)
//    }
 // t * sum(i*Dt)volts - (soc/100) *(soc /100) * (4.2 - 2.5)((100 - soc) / 100) ^ 2 * Dr * 100 -soc 
   /* print Batt1 info */
   b := Bats[0]
   b.discharge(100 ,10)
   fmt.Printf( "Batt1 name : %s soc %f volt %f\n", b.name, b.soc(), b.volts())
   b.discharge(100 ,10)
   fmt.Printf( "Batt1 name : %s soc %f volt %f\n", b.name, b.soc(), b.volts())
   /* print Batt2 info */
   fmt.Printf( "Tvolts : %f\n", tvolts(Bats[:]))

}