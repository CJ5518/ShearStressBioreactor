
//Common function to disallow scroll wheel editing number fields
//Because this feature is very rarely used intentionally, but often used unintentionally
document.addEventListener("wheel", function(event){
    if(document.activeElement.type === "number"){
        document.activeElement.blur();
    }
});
