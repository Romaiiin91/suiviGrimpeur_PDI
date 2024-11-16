//###################################################################
//
// select_input_class( container, headerArr )
//   container = div-object which is used as container
//   headerArr = Array with name of each column name
//
// Properties
// ----------
// disabled      = disables select so only scrollbar is usable
// selectedIndex = current selected index or array of indexes.
//                 -1 if unselected
// selectedValue = current selected value or array of values.
//                 null if unselected
// length        = number of rows
// width         = width of select, any valid css-width-value
//                 -1 if no with set
// multiselect   = true if multiple rows is allowed to be selected
//                 false if only one row is allowed to be selected
// onRowSelect   = eventhandler when selecting row
// ondblclick    = eventhandler when double-clicking row
//
// Methods
// -------
// addRow()      = Adds a new row
//     cellArray = array containing all text/objects
//     value     = value on row
//     title     = tooltip shown on mouseover
// clear()       = Clears values used for resetting internal values
// clearTable()  = Clears all rows in table, not internaly
// drawTable()   =   // Creates table and draws rows
// drawRows()    = Draw all rows
// removeRow()   = Removes a specified row
//     index     = row number to remove
// setColumnWidths() = Sets column width array
// setSortByColumn() = Sets which column should be sorted
//     index     = index of column to sort (0 based)
//                 -1 if not sorted
//     isDecending = true if sorting decending
//###################################################################

_select_input_class_sortByColumn = -1;
_select_input_class_sortDecending = true;
function select_input_class(container, headerArr)
{
  this._columnWidthArr = new Array();
  this.disabled = false;
  this._headers = (!headerArr ? new Array() : headerArr);
  this._rows = new Array();
  this.selectedIndex = -1;
  this.selectedValue = null;
  this._container = container;
  this._table;
  this._sortByColumn = -1;
  this._sortDecending = true;
  this.length = 0;
  this.width = -1;
  this.multiselect = false;
  this.onRowSelect = function (index){};
  this.ondblclick = function (index){};
  
  //internal handling when selecting row
  this._onRowSelect = function(index, e)
  {
    if (this.disabled)
    {
      return;
    }

    var evt = window.event || e;
    var ctrlPressed = evt.ctrlKey;
    var indexes = new Array();
    if( typeof( this.selectedIndex ) == "number"  )
      indexes.push( this.selectedIndex );
    else
      indexes = this.selectedIndex;

    //unselect current
    if (this.selectedIndex != -1 && this.multiselect && ctrlPressed)
    {
      var unselectedRow = false;
      for( var i = 0; i < indexes.length; i++ )
      {
        if( indexes[i] == index )
        {
          unselectedRow = true;
          this._rows[indexes[i]].isSelected = false;
        }
      }
      if( !unselectedRow )
      {
        this._rows[index].isSelected = true;
      }
    }
    else
    {
      if( indexes[0] >= 0 )
      {
        for( var i = 0; i < indexes.length; i++ )
        {
          this._rows[indexes[i]].isSelected = false;
        }
      }
      this._rows[index].isSelected = true;
    }
    this._updateSelected();
    if (this.onRowSelect)
    {
      this.onRowSelect(index);
    }
  }

  // used when doubleclicking row
  this._ondblclick = function(index)
  {
    if (this.disabled)
    {
      return;
    }
    if (this.ondblclick)
    {
      this.ondblclick(index);
    }
  }

  this.addRow = function(cellArray, value, title)
  {
    this.length++;
    this._rows.push(new select_input_row_class(cellArray, value, title));
  }

  this.clear = function()
  {
    this.selectedIndex = -1;
    this.selectedValue = null;
    this._rows.length = 0;
    this.length = 0;
  }

  this.clearTable = function()
  {
    while (this._table.tBodies[0] && this._table.tBodies[0].rows.length > 0)
    {
      this._table.tBodies[0].deleteRow(0);
    }
  }

  this.drawTable = function()
  {
    if (!this._table)
    {
      this._table = document.createElement("table");
      this._table.className = "selectList";
      this._container.appendChild(this._table);
      this._drawTableWidth();
      this._drawHeader();
    }
    this._sort();
    this.clearTable();
    this.drawRows();
  }
  
  // Internal draw width of table
  this._drawTableWidth = function()
  {
    if (this._columnWidthArr.length > 0)
    {
      var col;
      var colGroup = document.createElement("colgroup");

      for(var i = 0; i < this._columnWidthArr.length; i++)
      {
        col = document.createElement("COL");
        if (this._columnWidthArr[i])
        {
          col.style.width = this._columnWidthArr[i];
        }
        colGroup.appendChild(col);
      }
      this._table.appendChild(colGroup);
    }
  }

  // Internal draw headers
  this._drawHeader = function()
  {
    if (this._headers.length > 0)
    {
      this._createRow(this._headers, true);
    }
  }
  
  this.drawRows = function()
  {
    if (this._rows.length > 0)
    {
      var newRow;
      for(var i = 0; i < this._rows.length; i++)
      {
        newRow = this._createRow(this._rows[i].cellArray, false);
        this._rows[i].trObject = newRow;
        newRow._row_index = i;
        newRow._table = this;
        newRow.onclick = function(e){this._table._onRowSelect(this._row_index,e);};
        newRow.ondblclick = function(){this._table._ondblclick(this._row_index);};
        if (this._rows[i].title && this._rows[i].title.length > 0)
        {
          newRow.title = this._rows[i].title;
        }
        if (this._rows[i].isSelected)
        {
          newRow.className = "selected";
        }
      }
    }
  }
  
  this.removeRow = function(index)
  {  // Removes a specified row
  //   index = row number to remove

    if (index < 0 || index >= this.length)
    {
      return false;
    }
    this.length--;
    this._rows.splice(index, 1);
    this._updateSelected();
    this.drawTable();
  }
  
  this.setColumnWidths = function(widthArr)
  {
    this._columnWidthArr = widthArr;
  }
  
  this.setSortByColumn = function(index, isDecending)
  {
    this._sortByColumn = index;
    this._sortDecending = (arguments.lengt == 1 ? this._sortDecending : (isDecending ? true : false));
  }
  
  // Internal sort function
  this._sort = function()
  {
    if (this._sortByColumn >= 0)
    {
      _select_input_class_sortByColumn = this._sortByColumn;
      _select_input_class_sortDecending = this._sortDecending;
      this._rows.sort(this._sortByName);
    }
  }
  
  // Internal function used by Array().sort()
  this._sortByName = function(a, b)
  {
    var a_compare = "";
    var b_compare = ""
    try
    {
      a_compare = a.cellArray[_select_input_class_sortByColumn].toLowerCase();
    } catch (e){}
    try
    {
      b_compare = b.cellArray[_select_input_class_sortByColumn].toLowerCase();
    } catch (e){}
    return ( (_select_input_class_sortDecending ? -1 : 1) * ( a_compare ).localeCompare( b_compare ) );
  }

  // Internal creation of rows
  this._createRow = function(cellArr, isHeader)
  {
    var newRow, newCell;
    if (isHeader)
    {
      var thead = this._table.tHead;
      if (!thead)
      {
        thead = document.createElement("THEAD");
        this._table.appendChild(thead);
      }
      newRow = thead.insertRow(thead.rows.length);
    }
    else
    {
      var tbody = this._table.tBodies[0];
      if (!tbody)
      {
        tbody = document.createElement("TBODY");
        if (this.width > 0)
        {
          this._table.style.width = this.width;
        }
        this._table.appendChild(tbody);
      }
      newRow = tbody.insertRow(tbody.rows.length);
      newRow.className = "unselected";
    }
    for( var i = 0; i < cellArr.length; i++)
    {
      newCell = newRow.insertCell(i);
      newCell.unselectable = "on";
      newCell.innerHTML = cellArr[i];
      newCell._table = this;
    }
    return newRow;
  }

  // Updates selectedIndex, selectedValue and sets class of row
  this._updateSelected = function()
  {
    var newIndexes = new Array();
    var newValues = new Array();
    for( var i = 0; i < this.length; i++ )
    {
      if( this._rows[i].isSelected )
      {
        newIndexes.push(i);
        newValues.push(this._rows[i].value);
        this._rows[i].trObject.className = "selected";
      }
      else
      {
        this._rows[i].trObject.className = "unselected";
      }
    }
    if( newIndexes.length == 0 )
    {
      this.selectedIndex = -1;
      this.selectedValue = null;
    }
    else if( newIndexes.length == 1 )
    {
      this.selectedIndex = newIndexes[0];
      this.selectedValue = newValues[0];
    }
    else
    {
      this.selectedIndex = newIndexes;
      this.selectedValue = newValues;
    }
  }
}

function select_input_row_class(cellArray, value, title)
{
  this.value = value;
  this.cellArray = cellArray;
  this.title = title;
  this.isSelected = false;
  this.trObject;
}

