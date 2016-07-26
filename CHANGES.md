
## History Of Changes

* **2016-07-25 v0.0.12**
  - Equations are now extracted and represented as `IEEEeqnarray`

* **2016-07-16 v0.0.11**
  - LaTeX special characters are now escaped
  - Table content containing more than one paragraph is now handled
    correctly

* **2016-02-12 v0.0.10** The parser is now capable of detecting table
  captions. If the next paragraph after an table in an ODT file has the
style *Table* assigned it will be inserted as the value for the
``\caption`` TeX instruction.

