import pytest

from src.hello.hello import add

def test_add():
    assert add(5, 3) == 8
