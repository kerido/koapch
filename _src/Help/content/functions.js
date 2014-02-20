function HandleHeaderFooter()
{
	//1 Header
	var aNewHtml = 
	"<img class='LogoImage' src='../images/header_logo.gif' alt='KO Software Logo' />" +
	"<div class='Header'>" +
	"	<div class='HeaderStatic'>KO Approach Documentation</div>" +
	"	<h1>" + document.title + "</h1>" +
	"</div>"

	//2. Content
	aNewHtml += document.body.innerHTML;

	//3. Footer
	aNewHtml +=
	"<div class='Footer'>July, 2011, Rev. 05<br/>&copy;2004-2011 KO Software. All rights reserved.</div>";

	document.body.innerHTML = aNewHtml;
}

document.body.onload = HandleHeaderFooter;