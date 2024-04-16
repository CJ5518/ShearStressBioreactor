
console.log("hello, world!");

var firstEntry = document.getElementsByClassName("routineManagerEntry")[1];
var entryList = document.getElementById("entries");
var timeText = document.getElementById("routineTime");
var numEntries = 1;

function makeNewEntry() {
	var clone = firstEntry.cloneNode(true);
	clone.id = "entry" + numEntries;
	entryList.appendChild(clone);
	setNumbers(numEntries);
	numEntries++;
}

function removeEntry() {
	if (numEntries == 1) return;
	document.getElementById("entry" + (numEntries - 1)).remove();
	numEntries--;
}

function getNumbers(entryNum) {
	var ret = [];
	for (var q = 0; q < 4; q++) {
		ret[q] = parseFloat(document.querySelectorAll("#entry" + entryNum + " input")[q].value);
	}
	return ret;
}

function getAllNumbers() {
	var ret = [];
	for (var q = 0; q < numEntries; q++) {
		ret[q] = getNumbers(q);
	}
	return ret;
}

function setNumbers(entryNum) {
	for (var q = 0; q < 4; q++) {
		document.querySelectorAll("#entry" + entryNum + " input")[q].value = "";
	}
}

//https://stackoverflow.com/a/8211778
function secondsToString(seconds) {
	var numdays = Math.floor(seconds / 86400); 
	var numhours = Math.floor((seconds % 86400) / 3600);
	var numminutes = Math.floor(((seconds % 86400) % 3600) / 60);
	var numseconds = ((seconds % 86400) % 3600) % 60;
	return numdays + " days " + numhours + " hours " + numminutes + " minutes " + numseconds + " seconds";
}

//This function triggers every time an input on the page is edited
//We use this to update the routine time text
addEventListener('input', function (evt) {
	var text = "Total routine time: "
	var totalMillis = 0;
	var numbers = getAllNumbers();
	for (var q = 0; q < numbers.length; q++) {
		var millis = (numbers[q][1] + numbers[q][2]) * numbers[q][3];
		totalMillis += millis;
	}
	text += secondsToString(totalMillis / 1000);
	timeText.textContent = text;
});

function executeRoutine() {
	var numbers = getAllNumbers();
	var data = "";
	data += "numEntries=" + numbers.length;
	for (var q = 0; q < numbers.length; q++) {
		data += "&";
		for (var i = 0; i < numbers[q].length; i++) {
			data += ("entry" + q) + i;
			data += "=" + numbers[q][i];
			if (i < numbers[q].length-1) {
				data += "&";
			}
		}
	}

	var xhr = new XMLHttpRequest();

	xhr.onreadystatechange = function () {
		if (this.readyState != 4) return;

		if (this.status == 200) {
			console.log(this.responseText);
		}
	};

	xhr.open('POST', "./executeRoutine", true);
	xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
	console.log(data);
	xhr.send(data);
}
