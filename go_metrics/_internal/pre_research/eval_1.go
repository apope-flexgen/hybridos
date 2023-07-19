package research

// doc https://github.com/novalagung/golpal
import (
	"fmt"

	"github.com/novalagung/golpal"
)

var MyNum int

func main() {
	MyNum = 2
	if false {
		fmt.Printf("%d", MyNum)
	}
	//cmdString := `3 + 2`
	// cmdString := `
	// number := 3
	// if number == MyNum {
	// 	return "wrong"
	// } else {
	// 	return "right"
	// }
	// `

	// output, err := golpal.New().ExecuteSimple(cmdString)
	cmdString := `
	osName := runtime.GOOS
	arr := []string{"my", "operation system", "is", osName}
	return strings.Join(arr, ", ")
    `

	output, err := golpal.New().AddLibs("strings", "runtime").ExecuteSimple(cmdString)
	if err != nil {
		fmt.Println(err)
	}
	fmt.Println("result", "=>", output)
}
