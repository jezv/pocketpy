from typing import Callable, Any, Generic, TypeVar, Literal, overload

T = TypeVar('T')

Neighborhood = Literal['moore', 'von_neumann']

class array2d(Generic[T]):
    def __init__(self, n_cols: int, n_rows: int, default=None): ...
    @property
    def width(self) -> int: ...
    @property
    def height(self) -> int: ...
    @property
    def numel(self) -> int: ...

    def is_valid(self, col: int, row: int) -> bool: ...

    def get(self, col: int, row: int, default=None): ...

    @overload
    def __getitem__(self, index: tuple[int, int]): ...
    @overload
    def __getitem__(self, index: tuple[slice, slice]) -> 'array2d[T]': ...
    @overload
    def __setitem__(self, index: tuple[int, int], value: T): ...
    @overload
    def __setitem__(self, index: tuple[slice, slice], value: int | float | str | bool | None | 'array2d[T]'): ...

    def __len__(self) -> int: ...
    def __eq__(self, other: 'array2d') -> bool: ...
    def __ne__(self, other: 'array2d') -> bool: ...
    def __repr__(self): ...

    def tolist(self) -> list[list[T]]: ...

    def map(self, f: Callable[[T], Any]) -> 'array2d': ...
    def copy(self) -> 'array2d[T]': ...

    def fill_(self, value: T) -> None: ...
    def apply_(self, f: Callable[[T], T]) -> None: ...
    def copy_(self, other: 'array2d[T] | list[T]') -> None: ...

    # algorithms
    def count(self, value: T) -> int:
        """Counts the number of cells with the given value."""

    def count_neighbors(self, value: T, neighborhood: Neighborhood = 'moore') -> 'array2d[int]':
        """Counts the number of neighbors with the given value for each cell."""

    def find_bounding_rect(self, value: T) -> tuple[int, int, int, int] | None:
        """Finds the bounding rectangle of the given value.
        
        Returns a tuple `(x, y, width, height)` or `None` if the value is not found.
        """
