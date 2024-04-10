
console.log("hello, world!");

var firstEntry = document.getElementsByClassName("routineManagerEntry")[0];
var entryList = document.getElementById("entries");
var numEntries = 1;

function makeNewEntry() {
	var clone = firstEntry.cloneNode(true);
	clone.id = "entry" + numEntries;
	entryList.appendChild(clone);
	setNumbers(numEntries);
	numEntries++;
}

function getNumbers(entryNum) {
	var ret = [];
	for (var q = 0; q < 4; q++) {
		ret[q] = parseFloat(document.querySelectorAll("#entry" + entryNum + " input")[q].value);
	}
	return ret;
}

function setNumbers(entryNum) {
	for (var q = 0; q < 4; q++) {
		document.querySelectorAll("#entry" + entryNum + " input")[q].value = "";
	}
}
