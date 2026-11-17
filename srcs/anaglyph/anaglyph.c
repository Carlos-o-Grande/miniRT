/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   anaglyph.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cjoao-de <cjoao-de@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/14 19:45:50 by cjoao-de          #+#    #+#             */
/*   Updated: 2025/04/16 23:53:14 by cjoao-de         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <minirt.h>

static unsigned int	set_pixel_value(t_pixel l, t_pixel r)
{
	t_pixel	out;

	out.r = (int)(dlm(0, 0) * l.r + dlm(0, 1) * l.g
			+ dlm(0, 2) * l.b + drm(0, 0) * r.r
			+ drm(0, 1) * r.g + drm(0, 2) * r.b);
	out.g = (int)(dlm(1, 0) * l.r + dlm(1, 1) * l.g
			+ dlm(1, 2) * l.b + drm(1, 0) * r.r
			+ drm(1, 1) * r.g + drm(1, 2) * r.b);
	out.b = (int)(dlm(2, 0) * l.r + dlm(2, 1) * l.g
			+ dlm(2, 2) * l.b + drm(2, 0) * r.r
			+ drm(2, 1) * r.g + drm(2, 2) * r.b);
	out.r = clamp(out.r, 0, 255);
	out.g = clamp(out.g, 0, 255);
	out.b = clamp(out.b, 0, 255);
	return ((out.r << 16) | (out.g << 8) | out.b);
}

static void	dubois(t_minirt *s, unsigned int *red_ptr, \
		unsigned int *cyan_ptr, unsigned int *anaglyph_ptr)
{
	int				i;
	t_pixel			l;
	t_pixel			r;

	i = -1;
	l = (t_pixel){0, 0, 0, 0};
	r = (t_pixel){0, 0, 0, 0};
	while (++i < s->scene.cam.pixels)
	{
		l.r = (cyan_ptr[i] & 0x00FF0000) >> 16;
		l.g = (cyan_ptr[i] & 0x0000FF00) >> 8;
		l.b = (cyan_ptr[i] & 0x000000FF);
		r.r = (red_ptr[i] & 0x00FF0000) >> 16;
		r.g = (red_ptr[i] & 0x0000FF00) >> 8;
		r.b = (red_ptr[i] & 0x000000FF);
		anaglyph_ptr[i] = set_pixel_value(l, r);
	}
}

void	create_anaglyph(t_minirt *s)
{
	unsigned int	*red_ptr;
	unsigned int	*cyan_ptr;
	unsigned int	*anaglyph_ptr;

	red_ptr = (unsigned int *)s->scene.cam.red;
	cyan_ptr = (unsigned int *)s->scene.cam.cyan;
	anaglyph_ptr = (unsigned int *)s->scene.cam.anaglyph;
	ft_printf("Anaglyph generation starting - Dubois algorithm\n");
	create_right_eye(s);
	create_left_eye(s);
	dubois(s, red_ptr, cyan_ptr, anaglyph_ptr);
	dup_image(s->scene.cam.img.data, s->scene.cam.anaglyph);
	ft_printf("Dubois finished - anaglyph created\n");
}

// Apply depth-aware horizontal pixel shift
static void	apply_depth_modulated_shift(t_minirt *s, unsigned int *red_ptr,
		unsigned int *cyan_ptr)
{
	int				i;
	int				x;
	int				y;
	float			red_min;
	float			red_max;
	float			cyan_min;
	float			cyan_max;
	float			depth_norm;
	int				shift_amount;
	int				new_x;
	int				j;
	int				dst_idx;
	unsigned int	*temp_red;
	unsigned int	*temp_cyan;
	int				shift_count_red;
	int				shift_count_cyan;

	// Find min/max in log space
	red_min = INFINITY;
	red_max = 0.0f;
	cyan_min = INFINITY;
	cyan_max = 0.0f;
	j = -1;
	while (++j < s->scene.cam.pixels)
	{
		if (s->scene.cam.z_buffer_red[j] > 0.0f)
		{
			red_min = fminf(red_min, logf(s->scene.cam.z_buffer_red[j]));
			red_max = fmaxf(red_max, logf(s->scene.cam.z_buffer_red[j]));
		}
		if (s->scene.cam.z_buffer_cyan[j] > 0.0f)
		{
			cyan_min = fminf(cyan_min, logf(s->scene.cam.z_buffer_cyan[j]));
			cyan_max = fmaxf(cyan_max, logf(s->scene.cam.z_buffer_cyan[j]));
		}
	}
	printf("Depth-modulated shift: red_min=%.2f red_max=%.2f cyan_min=%.2f cyan_max=%.2f\n",
		red_min, red_max, cyan_min, cyan_max);

	// If no valid z-buffers, bail out
	if (red_min == INFINITY || cyan_min == INFINITY || red_max <= 0.0f || cyan_max <= 0.0f)
	{
		printf("WARNING: No valid z-buffer depth found (red_min=%f, cyan_min=%f), skipping shift\n",
			red_min, cyan_min);
		return ;
	}

	// Allocate temp buffers for shifted images
	temp_red = (unsigned int *)ft_calloc(s->scene.cam.pixels, sizeof(unsigned int));
	temp_cyan = (unsigned int *)ft_calloc(s->scene.cam.pixels, sizeof(unsigned int));
	if (!temp_red || !temp_cyan)
	{
		free(temp_red);
		free(temp_cyan);
		return ;
	}

	// Copy originals
	ft_memcpy(temp_red, red_ptr, s->scene.cam.pixels * sizeof(unsigned int));
	ft_memcpy(temp_cyan, cyan_ptr, s->scene.cam.pixels * sizeof(unsigned int));

	// DO NOT clear buffers - we need to preserve sky pixels (0x000000FF = blue)

	shift_count_red = 0;
	shift_count_cyan = 0;

	// Shift RED image to the RIGHT based on depth
	i = -1;
	while (++i < s->scene.cam.pixels)
	{
		y = i / W;
		x = i % W;

		if (s->scene.cam.z_buffer_red[i] > 0.0f)
		{
			depth_norm = (logf(s->scene.cam.z_buffer_red[i]) - red_min) / (red_max - red_min);
			depth_norm = powf(depth_norm, 0.5f);
			depth_norm = 1.0f - depth_norm;
			shift_amount = (int)(depth_norm * 6.0f);
			new_x = x + shift_amount;
			if (new_x >= 0 && new_x < W)
			{
				dst_idx = y * W + new_x;
				red_ptr[dst_idx] = temp_red[i];
				shift_count_red++;
			}
			else
			{
				// Out of bounds, keep original position
				red_ptr[i] = temp_red[i];
			}
		}
		// Sky pixels (z <= 0) stay where they are, keep temp_red values
	}

	// Shift CYAN image to the LEFT based on depth
	i = -1;
	while (++i < s->scene.cam.pixels)
	{
		y = i / W;
		x = i % W;

		if (s->scene.cam.z_buffer_cyan[i] > 0.0f)
		{
			depth_norm = (logf(s->scene.cam.z_buffer_cyan[i]) - cyan_min) / (cyan_max - cyan_min);
			depth_norm = powf(depth_norm, 0.5f);
			depth_norm = 1.0f - depth_norm;
			shift_amount = (int)(depth_norm * 6.0f);
			new_x = x - shift_amount;
			if (new_x >= 0 && new_x < W)
			{
				dst_idx = y * W + new_x;
				cyan_ptr[dst_idx] = temp_cyan[i];
				shift_count_cyan++;
			}
			else
			{
				// Out of bounds, keep original position
				cyan_ptr[i] = temp_cyan[i];
			}
		}
		// Sky pixels (z <= 0) stay where they are, keep temp_cyan values
	}

	free(temp_red);
	free(temp_cyan);
	printf("Depth-modulated shift: moved red=%d cyan=%d pixels\n",
		shift_count_red, shift_count_cyan);
}

void	create_anaglyph_depth(t_minirt *s)
{
	unsigned int	*red_ptr;
	unsigned int	*cyan_ptr;
	unsigned int	*anaglyph_ptr;

	red_ptr = (unsigned int *)s->scene.cam.red;
	cyan_ptr = (unsigned int *)s->scene.cam.cyan;
	anaglyph_ptr = (unsigned int *)s->scene.cam.anaglyph;
	ft_printf("Anaglyph depth generation starting - depth-modulated stereo\n");
	create_right_eye_with_z_buffer(s);
	calc_viewport_info(&s->scene);
	create_left_eye_with_z_buffer(s);
	calc_viewport_info(&s->scene);
	ft_printf("Applying depth-modulated shift...\n");
	apply_depth_modulated_shift(s, red_ptr, cyan_ptr);
	dubois(s, red_ptr, cyan_ptr, anaglyph_ptr);
	dup_image(s->scene.cam.img.data, s->scene.cam.anaglyph);
	ft_printf("Pre-computing z-buffer visualizations...\n");
	visualize_z_buffer_red(s);
	visualize_z_buffer_cyan(s);
	ft_printf("Depth-modulated anaglyph created\n");
}
