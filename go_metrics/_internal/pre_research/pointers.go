package research

import (
	"fmt"
)

type Company struct {
	Name    string
	Workers []*worker
}

type worker struct {
	Name  string
	Other []int
}

func (cmp *Company) NewWorker(name string) *worker {
	wrk := worker{Name: name}
	cmp.Workers = append(cmp.Workers, &wrk)
	return &wrk
}

func main() {
	cmp := Company{}
	cmp.Name = "Acme"
	wrk := cmp.NewWorker("Bugs")
	fmt.Println(&cmp)
	fmt.Println(&wrk)
	for i := 1; i <= 10; i++ {
		wrk.Other = append(wrk.Other, i)
	}
	fmt.Println(wrk)
	fmt.Println(&cmp)
	fmt.Println(&wrk)
	fmt.Println(cmp)
	fmt.Println(cmp.Workers[0])
}
