// Client-side JavaScript for the HTML server
async function loadStatus() {
    const statusResult = document.getElementById('status-result');
    const button = document.querySelector('button[onclick="loadStatus()"]');
    
    // Show loading state
    button.textContent = 'Loading...';
    button.disabled = true;
    
    try {
        const response = await fetch('/api/status');
        
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const data = await response.json();
        
        // Display the status information
        statusResult.innerHTML = `
            <h3>Server Status: ${data.status.toUpperCase()}</h3>
            <p><strong>Server:</strong> ${data.server}</p>
            <p><strong>Timestamp:</strong> ${new Date(data.timestamp * 1000).toLocaleString()}</p>
            <p><strong>Connection:</strong> Secure HTTPS ✅</p>
        `;
        
        statusResult.classList.add('show');
        
    } catch (error) {
        statusResult.innerHTML = `
            <h3>Error Loading Status</h3>
            <p style="color: #e53e3e;">Failed to connect to server: ${error.message}</p>
        `;
        statusResult.style.borderLeftColor = '#e53e3e';
        statusResult.classList.add('show');
    } finally {
        // Reset button state
        button.textContent = 'Check Server Status';
        button.disabled = false;
    }
}

// Add some interactive elements when page loads
document.addEventListener('DOMContentLoaded', function() {
    // Add click animations to buttons
    const buttons = document.querySelectorAll('.btn');
    buttons.forEach(button => {
        button.addEventListener('click', function(e) {
            // Create ripple effect
            const ripple = document.createElement('span');
            ripple.style.position = 'absolute';
            ripple.style.borderRadius = '50%';
            ripple.style.background = 'rgba(255,255,255,0.3)';
            ripple.style.transform = 'scale(0)';
            ripple.style.animation = 'ripple 0.6s linear';
            ripple.style.left = (e.clientX - button.offsetLeft) + 'px';
            ripple.style.top = (e.clientY - button.offsetTop) + 'px';
            
            button.style.position = 'relative';
            button.style.overflow = 'hidden';
            button.appendChild(ripple);
            
            setTimeout(() => {
                ripple.remove();
            }, 600);
        });
    });
    
    // Add CSS for ripple animation
    const style = document.createElement('style');
    style.textContent = `
        @keyframes ripple {
            to {
                transform: scale(4);
                opacity: 0;
            }
        }
    `;
    document.head.appendChild(style);
    
    console.log('🔒 Secure HTML Server loaded successfully!');
    console.log('Server features: HTTPS, HTTP redirect, static files, API endpoints');
});
