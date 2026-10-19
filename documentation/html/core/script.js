// When the user scrolls down 20px from the top of the document, show the button
window.onscroll = function() {ScrollFunction()};

function ScrollFunction() {
    if (document.body.scrollTop > 20 || document.documentElement.scrollTop > 20) {
        document.getElementById("buttonToTop").style.display = "block";
    } else {
        document.getElementById("buttonToTop").style.display = "none";
    }
}

// When the user clicks on the button, scroll to the top of the document
function PageToTop() {
    window.scrollTo({ top: 0, behavior: "smooth" });
}
