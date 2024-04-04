//import Chart from "./chart.js";

const data = [
		{ year: 1, count: 10 },
		{ year: 2, count: 20 },
		{ year: 3, count: 15 },
		{ year: 4, count: 25 },
		{ year: 5, count: 22 },
		{ year: 6, count: 30 },
		{ year: 7, count: 28 },
];

console.log("Boutta do charts!");

const xValues = [50,60,70,80,90,100,110,120,130,140,150];
const yValues = [7,8,8,9,9,9,10,11,14,14,15];

new Chart("mainChart", {
	type: "line",
	data: {
		labels: xValues,
		datasets: [{
			backgroundColor:"rgba(0,0,255,0)",
			borderColor: "rgba(0,0,255,0.9)",
			data: yValues
		}]
	},
	options: {
	legend: {
		display: false
	}
	}
});

console.log("Finished");
