# content of test_sample.py
import pytest

def func(x):
    return x + 1

def func2(x):
    return [x ,x*2,x*3]

def do_something():
    #Will be read from a file in real tests.
    #Path to the file is supplied via command line when running pytest
    list_int = [1, 2, 3, 4]
    list_res = [1, 2, 3, 5]

    for i in range(len(list_int)):
        if list_int[i] == list_res[i]:
            yield 4
        else:
            print(" value error")
            yield list_res[i]


@pytest.mark.parametrize('some_number', do_something())
def test_mt(some_number):
    assert(some_number == 4)

#def test_answer1():
#    assert func(3) == 5

#def test_answer2():
#    assert func(3) < 5

#def test_answer3():
#    assert func2(3)[1] == 6
#    assert func2(3)[2] == 19

#def test_answer4():
#    x = [2,3,4]
#    for xx in x:
#        print(xx)
#    assert func2(3)[1] == 6
#    assert func2(3)[2] == 19
