/*-chart on top stay--*/
$(window).on('scroll', function() {
	var scrollTop = $(this).scrollTop();

	$('.set_chart_space').each(function() {
		var topDistance = $(this).offset().top;

		if ( (topDistance+1) < scrollTop ) {
			$('.chart_space').addClass('show_nav');
		
		}else{
			$('.chart_space').removeClass('show_nav');
		  
		 }
	}); 
	
}); 
